#include "first_order_logic.h"
#include <algorithm>



template <typename T>
T concatLists(const T & c1, const T & c2)
{
    T c = c1;

    std::copy(c2.begin(), c2.end(), back_inserter(c));

    return c;
}

LiteralListList makePairs(const LiteralListList & c1, const LiteralListList & c2)
{
    LiteralListList c;

    for(auto & l1 : c1)
        for(auto & l2 : c2)
            c.push_back(concatLists(l1, l2));
    return c;
}

// DNF

LiteralListList True::listDNF()
{
    return {{}};
}

LiteralListList False::listDNF()
{
    return {};
}

LiteralListList Atom::listDNF()
{
    /* Pozitivan literal (atom) se predstavlja listom koja sadrzi
    samo jednu listu koja se sastoji iz tog jednog literala */
    return {{ shared_from_this() }};
}

LiteralListList Not::listDNF()
{
    /* Negativan literal se predstavlja listom koja sadrzi
    samo jednu listu koja se sastoji iz tog jednog literala */
    return {{ shared_from_this() }};
}

LiteralListList And::listDNF()
{
    /* DNF lista konjunkcije se dobija tako sto se DNF liste podformula
    distributivno "pomnoze", tj. liste literala se nadovezu svaka sa
    svakom */
    LiteralListList cl1 = _op1->listDNF();
    LiteralListList cl2 = _op2->listDNF();

    return makePairs(cl1, cl2);
}

LiteralListList Or::listDNF()
{
    /* DNF lista se kod disjunkcije dobija nadovezivanjem DNF listi
    podformula */
    LiteralListList cl1 = _op1->listDNF();
    LiteralListList cl2 = _op2->listDNF();

    return concatLists(cl1, cl2);
}

LiteralListList Imp::listDNF()
{
    throw "DNF not aplicable";
}

LiteralListList Iff::listDNF()
{
    throw "DNF not aplicable";
}

LiteralListList Quantifier::listDNF()
{
    throw "DNF not aplicable";
}

// Funkcije substitucije -----------------------------------------------

Term VariableTerm::substitute(const Variable & v, const Term & t)
{
    if(_v == v)
        return t;
    else
        return shared_from_this();
}

Term FunctionTerm::substitute(const Variable & v, const Term & t)
{
    vector<Term> sub_ops;

    for(unsigned i = 0; i < _ops.size(); i++) {
        sub_ops.push_back(_ops[i]->substitute(v, t));
    }

    return make_shared<FunctionTerm>(_sig, _f, sub_ops);
}

Formula LogicConstant::substitute(const Variable &, const Term &)
{
    return shared_from_this();
}

Formula Atom::substitute(const Variable & v, const Term & t)
{
    vector<Term> sub_ops;

    for(unsigned i = 0; i < _ops.size(); i++)
        sub_ops.push_back(_ops[i]->substitute(v, t));

    return make_shared<Atom>(_sig, _p, sub_ops);
}

Formula Not::substitute(const Variable & v, const Term & t)
{
    return make_shared<Not>(_op->substitute(v, t));
}

Formula And::substitute(const Variable & v, const Term & t)
{
    return make_shared<And>(_op1->substitute(v, t), _op2->substitute(v, t));
}

Formula Or::substitute(const Variable & v, const Term & t)
{
    return make_shared<Or>(_op1->substitute(v, t), _op2->substitute(v, t));
}

Formula Imp::substitute(const Variable & v, const Term & t)
{
    return make_shared<Imp>(_op1->substitute(v, t), _op2->substitute(v, t));
}

Formula Iff::substitute(const Variable & v, const Term & t)
{
    return make_shared<Iff>(_op1->substitute(v, t), _op2->substitute(v, t));
}


Formula Forall::substitute(const Variable & v, const Term & t)
{
    if(v == _v)
        return shared_from_this();

    /* Ako term sadrzi kvantifikovanu varijablu, tada moramo najpre
     preimenovati kvantifikovanu varijablu (nekom varijablom koja
     nije sadzana ni u termu ni u formuli sto nam daje funkcija
     getUniqueVariable koja je clanica klase Quantifier) */
    if(t->containsVariable(_v)) {
        Variable new_v = getUniqueVariable(shared_from_this(), t);
        Formula sub_op = _op->substitute(_v, make_shared<VariableTerm>(new_v));
        return make_shared<Forall>(new_v, sub_op->substitute(v, t));
    } else {
        return make_shared<Forall>(_v, _op->substitute(v, t));
    }
}

Formula Exists::substitute(const Variable & v, const Term & t)
{
    if(v == _v)
        return shared_from_this();

    /* Ako term sadrzi kvantifikovanu varijablu, tada moramo najpre
    preimenovati kvantifikovanu varijablu (nekom varijablom koja
    nije sadzana ni u termu ni u formuli sto nam daje funkcija
    getUniqueVariable koja je clanica klase Quantifier) */
    if(t->containsVariable(_v)) {
        Variable new_v = getUniqueVariable(shared_from_this(), t);

        Formula sub_op = _op->substitute(_v, make_shared<VariableTerm>(new_v));
        return make_shared<Exists>(new_v, sub_op->substitute(v, t));
    } else {
        return make_shared<Exists>(_v, _op->substitute(v, t));
    }
}

// ---------------------------------------------------------------------

// Funkcije za odredjivanje sintaksne identicnosti termova i formula ---

bool VariableTerm::equalTo(const Term & t) const
{
    return t->getType() == TT_VARIABLE &&
        ((VariableTerm *) t.get())->getVariable() == _v;
}

bool FunctionTerm::equalTo(const Term & t) const
{
    if(t->getType() != TT_FUNCTION)
        return false;

    if(_f != ((FunctionTerm *) t.get())->getSymbol())
        return false;

    const vector<Term> & t_ops = ((FunctionTerm *) t.get())->getOperands();

    if(_ops.size() != t_ops.size())
        return false;

    for(unsigned i = 0; i < _ops.size(); i++) {
        if(!_ops[i]->equalTo(t_ops[i])) {
            return false;
        }
    }

    return true;
}


bool LogicConstant::equalTo( const Formula & f) const
{
    return f->getType() == this->getType();
}


bool Atom::equalTo(const Formula & f) const
{
    if(f->getType() != T_ATOM)
        return false;

    if(_p != ((Atom *) f.get())->getSymbol())
        return false;

    const vector<Term> & f_ops = ((Atom *) f.get())->getOperands();

    if(_ops.size() != f_ops.size())
        return false;

    for(unsigned i = 0; i < _ops.size(); i++) {
        if(!_ops[i]->equalTo(f_ops[i])) {
            return false;
        }
    }

    return true;
}

bool UnaryConnective::equalTo(const Formula & f) const
{
    return f->getType() == this->getType() &&
        _op->equalTo(((UnaryConnective *)f.get())->getOperand());
}

bool BinaryConnective::equalTo( const Formula & f) const
{
    return f->getType() == this->getType() &&
        _op1->equalTo(((BinaryConnective *)f.get())->getOperand1()) &&
        _op2->equalTo(((BinaryConnective *)f.get())->getOperand2());
}

bool Quantifier::equalTo(const Formula & f) const
{
    return f->getType() == getType() &&
        ((Quantifier *) f.get())->getVariable() == _v &&
        ((Quantifier *) f.get())->getOperand()->equalTo(_op);
}

// ---------------------------------------------------------------------

// Funkcije za odredjivanje skupa varijabli i konstanti ----------------

void VariableTerm::getVars(VariableSet & vars) const
{
    vars.insert(_v);
}

void VariableTerm::getConstants(ConstantSet &) const
{
    return;
}

void VariableTerm::getFunctions(FunctionSet &) const
{
    return;
}

void FunctionTerm::getVars(VariableSet & vars) const
{
    for(unsigned i = 0; i < _ops.size(); i++) {
        _ops[i]->getVars(vars);
    }
}

void FunctionTerm::getConstants(ConstantSet & cts) const
{
    if (_ops.size() == 0) {
        cts.insert(_f);
    } else {
        for(unsigned i = 0; i < _ops.size(); i++) {
            _ops[i]->getConstants(cts);
        }
    }
}

void FunctionTerm::getFunctions(FunctionSet & fs) const
{
    if (_ops.size() != 0) {
        fs.insert(_f);
    }
    for(unsigned i = 0; i < _ops.size(); i++) {
        _ops[i]->getFunctions(fs);
    }
}

void LogicConstant::getVars(VariableSet &, bool) const
{
    return;
}

void LogicConstant::getConstants(ConstantSet &) const
{
    return;
}

void LogicConstant::getFunctions(FunctionSet &) const
{
    return;
}

void Atom::getVars(VariableSet & vars, bool) const
{
    for(unsigned i = 0; i < _ops.size(); i++) {
        _ops[i]->getVars(vars);
    }
}

void Atom::getConstants(ConstantSet & cts) const
{
    for(unsigned i = 0; i < _ops.size(); i++) {
        _ops[i]->getConstants(cts);
    }
}

void Atom::getFunctions(FunctionSet & fs) const
{
    for(unsigned i = 0; i < _ops.size(); i++) {
        _ops[i]->getFunctions(fs);
    }
}

void UnaryConnective::getVars(VariableSet & vars, bool free) const
{
    _op->getVars(vars, free);
}

void UnaryConnective::getConstants(ConstantSet & cts) const
{
    _op->getConstants(cts);
}

void UnaryConnective::getFunctions(FunctionSet & fs) const
{
    _op->getFunctions(fs);
}

void BinaryConnective::getVars(VariableSet & vars, bool free) const
{
    _op1->getVars(vars, free);
    _op2->getVars(vars, free);
}

void BinaryConnective::getConstants(ConstantSet & cts) const
{
    _op1->getConstants(cts);
    _op2->getConstants(cts);
}

void BinaryConnective::getFunctions(FunctionSet & fs) const
{
    _op1->getFunctions(fs);
    _op2->getFunctions(fs);
}

void Quantifier::getVars(VariableSet & vars, bool free) const
{
    bool present = false;

    if(free) {
        /* Pamtimo da li je kvantifikovana varijabla vec u skupu slobodnih
        varijabli */
        if(vars.find(_v) != vars.end()) {
            present = true;
        }
    }

    _op->getVars(vars, free);
    if(!free) {
        vars.insert(_v);
    } else {
        /* Ako varijabla ranije nije bila prisutna u skupu slobodnih varijabli,
        tada je brisemo, zato sto to znaci da se ona pojavljuje samo u
        podformuli kvantifikovane formule,a u njoj je vezana kvantifikatorom */
        if(!present && vars.find(_v) != vars.end()) {
            vars.erase(_v);
        }
    }
}

void Quantifier::getConstants(ConstantSet & cts) const
{
    _op->getConstants(cts);
}

void Quantifier::getFunctions(FunctionSet & fs) const
{
    _op->getFunctions(fs);
}

// ---------------------------------------------------------------------

// Funkcije za odredjivanje slozenosti formule -------------------------

unsigned AtomicFormula::complexity() const
{
    return 0;
}

unsigned UnaryConnective::complexity() const
{
    return _op->complexity() + 1;
}

unsigned BinaryConnective::complexity() const
{
    return _op1->complexity() + _op2->complexity() + 1;
}

unsigned Quantifier::complexity() const
{
    return _op->complexity() + 1;
}

// ---------------------------------------------------------------------

// Funkcije za stampanje -----------------------------------------------


void VariableTerm::printTerm(ostream & ostr) const
{
    ostr << _v;
}

void FunctionTerm::printTerm(ostream & ostr) const
{
    ostr << _f;

    for(unsigned i = 0; i < _ops.size(); i++) {
        if(i == 0) {
            ostr << "(";
        }

        ostr << _ops[i];

        if(i != _ops.size() - 1) {
            ostr << ",";
        }
        else {
            ostr << ")";
        }
    }
}

void True::printFormula(ostream & ostr) const
{
    ostr << "True";
}

void False::printFormula(ostream & ostr) const
{
    ostr << "False";
}

void Atom::printFormula(ostream & ostr) const
{
    ostr << _p;
    for(unsigned i = 0; i < _ops.size(); i++) {
        if(i == 0) {
            ostr << "(";
        }

        ostr << _ops[i];

        if(i != _ops.size() - 1) {
            ostr << ",";
        } else {
            ostr << ")";
        }
    }
}

void Not::printFormula(ostream & ostr) const
{
    ostr << "(~" << _op << ")";
}

void And::printFormula(ostream & ostr) const
{
    ostr << "(" << _op1 << " /\\ " << _op2 << ")";
}

void Or::printFormula(ostream & ostr) const
{
    ostr << "(" << _op1 << " \\/ " << _op2 << ")";
}

void Imp::printFormula(ostream & ostr) const
{
    ostr << "(" << _op1 << " ==> " << _op2 << ")";
}

void Iff::printFormula(ostream & ostr) const
{
    ostr << "(" << _op1 << " <=> " << _op2 << ")";
}

void Forall::printFormula(ostream & ostr) const
{
    ostr << "(A " << _v << ").(" << _op << ")";
}

void Exists::printFormula(ostream & ostr) const
{
    ostr << "(E " << _v << ").(" << _op << ")";
}


ostream & operator << (ostream & ostr, const Term & t)
{
    t->printTerm(ostr);
    return ostr;
}

ostream & operator << (ostream & ostr, const Formula & f)
{
    f->printFormula(ostr);
    return ostr;
}


// ---------------------------------------------------------------------


// Funkcije za izracunvanje interpretacija -----------------------------

unsigned VariableTerm::eval(const Structure &, const Valuation & val) const
{
    return val.getValue(_v);
}

unsigned FunctionTerm::eval(const Structure & st, const Valuation & val) const
{
    Function * f = st.getFunction(_f);

    vector<unsigned> args;

    for(unsigned i = 0; i < _ops.size(); i++) {
        args.push_back(_ops[i]->eval(st, val));
    }

    return f->eval(args);
}

bool True::eval(const Structure &, const Valuation &) const
{
    return true;
}

bool False::eval(const Structure &,  const Valuation &) const
{
    return false;
}

bool Atom::eval(const Structure & st, const Valuation & val) const
{
    Relation * r = st.getRelation(_p);

    vector<unsigned> args;

    for(unsigned i = 0; i < _ops.size(); i++) {
        args.push_back(_ops[i]->eval(st, val));
    }

    return r->eval(args);
}

bool Not::eval(const Structure & st, const Valuation & val) const
{
    return !_op->eval(st, val);
}

bool And::eval(const Structure & st, const Valuation & val) const
{
    return _op1->eval(st, val) && _op2->eval(st, val);
}


bool Or::eval(const Structure & st, const Valuation & val) const
{
    return _op1->eval(st, val) || _op2->eval(st, val);
}

bool Imp::eval(const Structure & st, const Valuation & val) const
{
    return !_op1->eval(st, val) || _op2->eval(st, val);
}

bool Iff::eval(const Structure & st, const Valuation & val) const
{
    return _op1->eval(st, val) == _op2->eval(st, val);
}

bool Forall::eval(const Structure & st, const Valuation & val) const
{
    Valuation val_p = val;
    for(unsigned i = 0; i < st.getDomain().size(); i++) {
        val_p.setValue(_v, st.getDomain()[i]);

        if(_op->eval(st, val_p) == false) {
            return false;
        }
    }
    return true;
}


bool Exists::eval(const Structure & st, const Valuation & val) const
{
    Valuation val_p = val;
    for(unsigned i = 0; i < st.getDomain().size(); i++) {
        val_p.setValue(_v, st.getDomain()[i]);

        if(_op->eval(st, val_p) == true){
            return true;
        }
    }
    return false;
}

// -----------------------------------------------------------------------

// Ostale funkcije clanice -----------------------------------------------

// Klasa Signature -------------------------------------------------------


Signature::Signature()
{
    _currUniq = 0;
    for (size_t i = 0; i < 100; ++i) {
        std::stringstream ss;
        ss << "uc" << i;
        _uniqueConstants.push_back(ss.str());
        addFunctionSymbol(ss.str(), 0);
    }
}

FunctionSymbol Signature::getNewUniqueConstant()
{
    return _uniqueConstants[_currUniq++];
}

void Signature::addFunctionSymbol(const FunctionSymbol & f, unsigned arity)
{
    _functions.insert(make_pair(f, arity));
}

void Signature::addPredicateSymbol(const PredicateSymbol & p, unsigned arity)
{
    _predicates.insert(make_pair(p, arity));
}

bool Signature::checkFunctionSymbol(const FunctionSymbol & f, unsigned & arity) const
{
    map<FunctionSymbol, unsigned>::const_iterator it = _functions.find(f);

    if(it != _functions.end()) {
        arity = it->second;
        return true;
    } else {
        return false;
    }
}

bool Signature::checkPredicateSymbol(const PredicateSymbol & f, unsigned & arity) const
{
    map<PredicateSymbol, unsigned>::const_iterator it = _predicates.find(f);

    if(it != _predicates.end()) {
        arity = it->second;
        return true;
    } else {
        return false;
    }
}

// -----------------------------------------------------------------------

// Klasa Function --------------------------------------------------------

Function::Function(unsigned arity)
    : _arity(arity)
{}

unsigned Function::getArity()
{
    return _arity;
}
// ----------------------------------------------------------------------

// Klasa Relation -------------------------------------------------------

Relation::Relation(unsigned arity)
    : _arity(arity)
{}

unsigned Relation::getArity()
{
    return _arity;
}
// ----------------------------------------------------------------------

// Klasa Structure ------------------------------------------------------

Structure::Structure(const Signature & sig, const Domain & domain)
    : _sig(sig), _domain(domain)
{}

const Signature & Structure::getSignature() const
{
    return _sig;
}

const Domain & Structure::getDomain() const
{
    return _domain;
}

void Structure::addFunction(const FunctionSymbol & fs, Function * f)
{
    unsigned arity;
    if(!_sig.checkFunctionSymbol(fs, arity) || arity != f->getArity()) {
        throw "Function arity mismatch";
    }

    _funs.insert(make_pair(fs, f));
}

Function * Structure::getFunction(const FunctionSymbol & f) const
{
    map<FunctionSymbol, Function *>::const_iterator it = _funs.find(f);

    if(it != _funs.end()) {
        return it->second;
    } else {
        throw "Function symbol unknown!";
    }
}

void Structure::addRelation(const PredicateSymbol & ps, Relation * r)
{
    unsigned arity;
    if(!_sig.checkPredicateSymbol(ps, arity) || arity != r->getArity()) {
        throw "Relation arity mismatch";
    }

    _rels.insert(make_pair(ps, r));
}

Relation * Structure::getRelation(const PredicateSymbol & p) const
{
    map<PredicateSymbol, Relation *>::const_iterator it = _rels.find(p);

    if(it != _rels.end()) {
        return it->second;
    } else {
        throw "Predicate symbol unknown!";
    }
}

Structure::~Structure()
{
    for(const auto & p : _funs) {
        delete p.second;
    }
    for(const auto & p : _rels) {
        delete p.second;
    }
}

// ----------------------------------------------------------------------

// Klasa Valuation ------------------------------------------------------

Valuation::Valuation(const Domain & dom)
    : _domain(dom)
{}

const Domain & Valuation::getDomain() const
{
    return _domain;
}

void Valuation::setValue(const Variable & v, unsigned value)
{
    if(find(_domain.begin(), _domain.end(), value) == _domain.end()) {
        throw "Value not in domain!";
    }

    _values[v] = value;
}

unsigned Valuation::getValue(const Variable & v) const
{
    map<Variable, unsigned>::const_iterator it = _values.find(v);

    if(it != _values.end()) {
        return it->second;
    } else {
        throw "Variable unknown!";
    }
}

// -----------------------------------------------------------------------

// Klasa BaseTerm ------------------------------------------------------------

bool BaseTerm::containsVariable(const Variable & v) const
{
    VariableSet vars;
    getVars(vars);
    return vars.find(v) != vars.end();
}

// -----------------------------------------------------------------------

// Klasa VariableTerm ----------------------------------------------------

VariableTerm::VariableTerm(const Variable & v)
    : _v(v)
{}

BaseTerm::Type VariableTerm::getType() const
{
    return TT_VARIABLE;
}

const Variable & VariableTerm::getVariable() const
{
    return _v;
}

// -----------------------------------------------------------------------

// Klasa FunctionTerm ----------------------------------------------------

FunctionTerm::FunctionTerm(const Signature & s, const FunctionSymbol & f,
        const vector<Term> & ops)
    : _sig(s), _f(f), _ops(ops)
{
    unsigned arity;
    if(!_sig.checkFunctionSymbol(_f, arity) || arity != _ops.size()) {
        throw "Syntax error!";
    }
}

FunctionTerm::FunctionTerm(const Signature & s, const FunctionSymbol & f,
        vector<Term> && ops)
    : _sig(s), _f(f), _ops(std::move(ops))
{
    unsigned arity;
    if(!_sig.checkFunctionSymbol(_f, arity) || arity != _ops.size()) {
        throw "Syntax error!";
    }
}

BaseTerm::Type FunctionTerm::getType() const
{
    return TT_FUNCTION;
}

const Signature & FunctionTerm::getSignature() const
{
    return _sig;
}

const FunctionSymbol & FunctionTerm::getSymbol() const
{
    return _f;
}

const vector<Term> & FunctionTerm::getOperands() const
{
    return _ops;
}

// ----------------------------------------------------------------------

// Klasa BaseFormula --------------------------------------------------------

bool BaseFormula::containsVariable(const Variable & v, bool free) const
{
    VariableSet vars;
    getVars(vars, free);
    return vars.find(v) != vars.end();
}

// ----------------------------------------------------------------------

template <typename T1, typename T2>
Variable getUniqueVariable(const T1 & e1, const T2 & e2)
{
    static unsigned i = 0;

    Variable v;

    do {
        v = string("uv") + to_string(++i);
    } while(e1->containsVariable(v) || e2->containsVariable(v));

    return v;
}

Formula removeUniversalQ(const Formula & f) {
    Forall * fa = NULL;
    if (f->getType() == BaseFormula::T_FORALL) {
        fa = (Forall*)f.get();
        return removeUniversalQ(fa->getOperand());
    } else {
        return f;
    }
}



FunctionSymbol getUniqueFunctionSymbol(const Signature & s)
{
    static unsigned i = 0;
    unsigned arity;

    FunctionSymbol f;

    do {
        f = string("uf") + to_string(++i);
    } while(s.checkFunctionSymbol(f, arity));

    return f;
}

// ----------------------------------------------------------------------

// Klasa AtomicFormula --------------------------------------------------

// Klasa LogicConstant --------------------------------------------------

BaseFormula::Type True::getType() const
{
    return T_TRUE;
}
// ----------------------------------------------------------------------

// Klasa False ----------------------------------------------------------

BaseFormula::Type False::getType() const
{
    return T_FALSE;
}

// ----------------------------------------------------------------------

// Klasa Atom -----------------------------------------------------------

Atom::Atom(const Signature & s, const PredicateSymbol & p, const vector<Term> & ops)
    : _sig(s), _p(p), _ops(ops)
{
    unsigned arity;
    if(!_sig.checkPredicateSymbol(_p, arity) || arity != _ops.size()) {
        throw "Syntax error!";
    }
}

Atom::Atom(const Signature & s, const PredicateSymbol & p, vector<Term> && ops)
    : _sig(s), _p(p), _ops(std::move(ops))
{
    unsigned arity;
    if(!_sig.checkPredicateSymbol(_p, arity) || arity != _ops.size()) {
        throw "Syntax error!";
    }
}

const PredicateSymbol & Atom::getSymbol() const
{
    return _p;
}

const Signature & Atom::getSignature() const
{
    return _sig;
}

const vector<Term> & Atom::getOperands() const
{
    return _ops;
}

BaseFormula::Type Atom::getType() const
{
    return T_ATOM;
}

// -----------------------------------------------------------------------

// Klasa UnaryConnective -------------------------------------------------

UnaryConnective::UnaryConnective(const Formula & op)
    : _op(op)
{}

const Formula & UnaryConnective::getOperand() const
{
    return _op;
}

// -----------------------------------------------------------------------

// Klasa Not -------------------------------------------------------------

BaseFormula::Type Not::getType() const
{
    return T_NOT;
}

// -----------------------------------------------------------------------

// Klasa BinaryConnective ------------------------------------------------

BinaryConnective::BinaryConnective( const Formula & op1,  const Formula & op2)
    : _op1(op1), _op2(op2)
{}

const Formula & BinaryConnective::getOperand1() const
{
    return _op1;
}

const Formula & BinaryConnective::getOperand2() const
{
    return _op2;
}

// Klasa And ---------------------------------------------------------------

BaseFormula::Type And::getType() const
{
    return T_AND;
}

// -------------------------------------------------------------------------

// Klasa Or ----------------------------------------------------------------

BaseFormula::Type Or::getType() const
{
    return T_OR;
}

// -------------------------------------------------------------------------

// Klasa Imp ---------------------------------------------------------------

BaseFormula::Type Imp::getType() const
{
    return T_IMP;
}

// -------------------------------------------------------------------------

// Klasa Iff ---------------------------------------------------------------

BaseFormula::Type Iff::getType() const
{
    return T_IFF;
}

// -------------------------------------------------------------------------

// Klasa Quantifier --------------------------------------------------------

Quantifier::Quantifier(const Variable & v, const Formula & op)
    : _v(v), _op(op)
{}

const Variable & Quantifier::getVariable() const
{
    return _v;
}

const Formula & Quantifier::getOperand() const
{
    return _op;
}

// ------------------------------------------------------------------------

// Klasa Forall -----------------------------------------------------------

BaseFormula::Type Forall::getType() const
{
    return T_FORALL;
}

// ------------------------------------------------------------------------

// Klasa Exists -----------------------------------------------------------

BaseFormula::Type Exists::getType() const
{
    return T_EXISTS;
}

// -----------------------------------------------------------------------


// Funkcije za simplifikaciju -------------------------------------------

/* Simplifikacija atomicke formule je trivijalna */
Formula AtomicFormula::simplify()
{
    return shared_from_this();
}

Formula Not::simplify()
{
    Formula simp_op = _op->simplify();

    if(simp_op->getType() == T_TRUE) {
        return make_shared<False>();
    } else if(simp_op->getType() == T_FALSE) {
        return make_shared<True>();
    } else {
        return make_shared<Not>(simp_op);
    }
}

Formula And::simplify()
{
    /* Simplifikacija konjukcije po pravilima A /\ True === A, 
    A /\ False === False i sl. */
    Formula simp_op1 = _op1->simplify();
    Formula simp_op2 = _op2->simplify();

    if(simp_op1->getType() == T_TRUE) {
        return simp_op2;
    } else if(simp_op2->getType() == T_TRUE) {
        return simp_op1;
    } else if(simp_op1->getType() == T_FALSE || simp_op2->getType() == T_FALSE) {
        return make_shared<False>();
    } else {
        return make_shared<And>(simp_op1, simp_op2);
    }
}

Formula Or::simplify()
{
    /* Simplifikacija disjunkcije po pravilima: A \/ True === True,
    A \/ False === A, i sl. */
    Formula simp_op1 = _op1->simplify();
    Formula simp_op2 = _op2->simplify();

    if(simp_op1->getType() == T_FALSE) {
        return simp_op2;
    } else if(simp_op2->getType() == T_FALSE) {
        return simp_op1;
    } else if(simp_op1->getType() == T_TRUE || simp_op2->getType() == T_TRUE) {
        return make_shared<True>();
    } else {
        return make_shared<Or>(simp_op1, simp_op2);
    }
}

Formula Imp::simplify()
{
    /* Simplifikacija implikacije po pravilima: A ==> True === True,
    A ==> False === ~A, True ==> A === A, False ==> A === True */
    Formula simp_op1 = _op1->simplify();
    Formula simp_op2 = _op2->simplify();

    if(simp_op1->getType() == T_FALSE) {
        return make_shared<True>();
    } else if(simp_op2->getType() == T_FALSE) {
        return make_shared<Not>(simp_op1);
    } else if(simp_op1->getType() == T_TRUE) {
        return simp_op2;
    } else if(simp_op2->getType() == T_TRUE) {
        return make_shared<True>();
    } else {
        return make_shared<Imp>(simp_op1, simp_op2);
    }
}

Formula Iff::simplify()
{
    /* Ekvivalencija se simplifikuje pomocu pravila:
    True <=> A === A, False <=> A === ~A i sl. */
    Formula simp_op1 = _op1->simplify();
    Formula simp_op2 = _op2->simplify();

    if(simp_op1->getType() == T_FALSE && simp_op2->getType() == T_FALSE) {
        return make_shared<True>();
    } else if(simp_op1->getType() == T_FALSE) {
        return make_shared<Not>(simp_op2);
    } else if(simp_op2->getType() == T_FALSE) {
        return make_shared<Not>(simp_op1);
    } else if(simp_op1->getType() == T_TRUE) {
        return simp_op2;
    } else if(simp_op2->getType() == T_TRUE) {
        return simp_op1;
    } else {
        return make_shared<Iff>(simp_op1, simp_op2);
    }
}

Formula Forall::simplify()
{
    Formula simp_op = _op->simplify();

    /* Ako simplifikovana podformula sadrzi slobodnu varijablu v, tada
    zadrzavamo kvantifikator, u suprotnom ga brisemo */
    if(simp_op->containsVariable(_v, true)) {
        return  make_shared<Forall>(_v, simp_op);
    } else {
        return simp_op;
    }
}

Formula Exists::simplify()
{
    Formula simp_op = _op->simplify();

    /* Ako simplifikovana podformula sadrzi slobodnu varijablu v, tada
    zadrzavamo kvantifikator, u suprotnom ga brisemo */
    if(simp_op->containsVariable(_v, true)) {
        return make_shared<Exists>(_v, simp_op);
    } else {
        return simp_op;
    }
}



// ---------------------------------------------------------------------


// NNF funkcije --------------------------------------------------------

Formula AtomicFormula::nnf()
{
    return shared_from_this();
}

Formula Not::nnf()
{
    if(_op->getType() == T_NOT) {
        /* Eliminacija dvojne negacije */
        Not * not_op = (Not *) _op.get();
        return not_op->getOperand()->nnf();
    } else if(_op->getType() == T_AND) {
        /* De-Morganov zakon ~(A/\B) === ~A \/ ~B, pa zatim rekurzivna
        primena nnf-a na ~A i ~B */
        And * and_op =  (And *) _op.get();

        return make_shared<Or>(make_shared<Not>(and_op->getOperand1())->nnf(),
            make_shared<Not>(and_op->getOperand2())->nnf());
    } else if(_op->getType() == T_OR) {
        /* De-Morganov zakon ~(A\/B) === ~A /\ ~B, pa zatim rekurzivna
        primena nnf-a na ~A i ~B */
        Or * or_op =  (Or *) _op.get();

        return make_shared<And>(make_shared<Not>(or_op->getOperand1())->nnf(),
            make_shared<Not>(or_op->getOperand2())->nnf());
    } else if(_op->getType() == T_IMP) {
        /* De-Morganov zakon ~(A==>B) === A /\ ~B, pa zatim rekurzivna
        primena nnf-a na A i ~B */
        Imp * imp_op =  (Imp *) _op.get();

        return make_shared<And>(imp_op->getOperand1()->nnf(),
            make_shared<Not>(imp_op->getOperand2())->nnf());
    } else if(_op->getType() == T_IFF) {
        /* Primena pravila ~(A<=>B) === (A /\ ~B) \/ (B /\ ~A) */
        Iff * iff_op =  (Iff *) _op.get();

        return make_shared<Or>(make_shared<And>(iff_op->getOperand1()->nnf(),
                  make_shared<Not>(iff_op->getOperand2())->nnf()),
            make_shared<And>(iff_op->getOperand2()->nnf(),
                  make_shared<Not>(iff_op->getOperand1())->nnf()));
    } else if(_op->getType() == T_FORALL) {
        /* Primena pravila ~(forall x) A === (exists x) ~A */
        Forall * forall_op = (Forall *) _op.get();

        return make_shared<Exists>(forall_op->getVariable(),
            make_shared<Not>(forall_op->getOperand())->nnf());
    } else if(_op->getType() == T_EXISTS) {
        /* Primena pravila ~(exists x) A === (forall x) ~A */
        Exists * exists_op = (Exists *) _op.get();

        return make_shared<Forall>(exists_op->getVariable(),
            make_shared<Not>(exists_op->getOperand())->nnf());
    } else {
        return shared_from_this();
    }
}

Formula And::nnf()
{
    return make_shared<And>(_op1->nnf(), _op2->nnf());
}

Formula Or::nnf()
{
    return make_shared<Or>(_op1->nnf(), _op2->nnf());
}

Formula Imp::nnf()
{
    /* Eliminacija implikacije, pa zatim rekurzivna primena nnf()-a */
    return make_shared<Or>(make_shared<Not>(_op1)->nnf(), _op2->nnf());
}

Formula Iff::nnf()
{
    /* Eliminacija ekvivalencije, pa zatim rekurzivna primena nnf()-a.
    Primetimo da se ovde velicina formule duplira */
    return make_shared<And>(make_shared<Or>(make_shared<Not>(_op1)->nnf(), _op2->nnf()),
        make_shared<Or>(make_shared<Not>(_op2)->nnf(), _op1->nnf()));
}

Formula Forall::nnf()
{
    return make_shared<Forall>(_v, _op->nnf());
}

Formula Exists::nnf()
{
    return make_shared<Exists>(_v, _op->nnf());
}

// Funkcije za izvlacenje kvantifikatora  ------------------------------

Formula AtomicFormula::pullquants()
{
    /* U slucaju atomicke formule, ne radimo nista */
    return shared_from_this();
}

Formula Not::pullquants()
{
    /* U slucaju negacije, ne radimo nista, s obzirom da je formula
    vec u NNF-u, pa su negacije spustene do nivoa atoma. */
    return shared_from_this();
}

Formula And::pullquants()
{
    /* Ako je formula oblika (forall x) A /\ (forall y) B */
    if(_op1->getType() == T_FORALL && _op2->getType() == T_FORALL) {
        Forall * fop1 = (Forall *) _op1.get();
        Forall * fop2 = (Forall *) _op2.get();

        /* Specijalno, ako je formula oblika (forall x) A /\ (forall x) B
        tada ne moramo da preimenujemo vezanu promenljivu, vec samo
        izvlacimo kvantifikator (forall x) (A /\ B), a zatim iz podformule
        rekurzivno izvucemo kvantifikatore */
        if(fop1->getVariable() == fop2->getVariable()) {
            return make_shared<Forall>(fop1->getVariable(),
                make_shared<And>(fop1->getOperand(),
                fop2->getOperand())->pullquants());
        } else {
            /* U suprotnom, uvodimo novu promenljivu koja se ne pojavljuje
            ni u jednoj od  formula A i B */
            Variable var = 
            getUniqueVariable(fop1->getOperand(), fop2->getOperand());

            return make_shared<Forall>(
                var,
                make_shared<And>(
                    fop1->getOperand()->substitute(
                        fop1->getVariable(), make_shared<VariableTerm>(var)
                    ),
                    fop2->getOperand()->substitute(
                        fop2->getVariable(), make_shared<VariableTerm>(var)
                    )
                )->pullquants()
            );
        }
    } else if(_op1->getType() == T_EXISTS) {
        /* Slucaj ((exists x) A) /\ B */
        Exists * eop1 = (Exists *) _op1.get();

        /* Ako x ne postoji kao slobodna varijabla u B, tada je dovoljno
        samo izvuci kvantifikator (exists x) (A /\ B) */
        if(!_op2->containsVariable(eop1->getVariable(), true)) {
            return make_shared<Exists>(
                eop1->getVariable(),
                make_shared<And>(
                    eop1->getOperand(), _op2
                )->pullquants()
            );
        } else {
            /* u suprotnom, moramo da preimenujemo vezanu varijablu */
            Variable var = getUniqueVariable(eop1->getOperand(), _op2);
                return make_shared<Exists>(var,
                    make_shared<And>(
                        eop1->getOperand()->substitute(
                            eop1->getVariable(), make_shared<VariableTerm>(var)
                        ), _op2)->pullquants());
        }
    } else if(_op2->getType() == T_EXISTS) {
        /* Slucaj A /\ (exists x) B */
        Exists * eop2 = (Exists *) _op2.get();

        /* Ako x ne postoji kao slobodna varijabla u A, tada je dovoljno
        samo izvuci kvantifikator (exists x) (A /\ B) */
        if(!_op1->containsVariable(eop2->getVariable(), true)) {
            return make_shared<Exists>(eop2->getVariable(),
                make_shared<And>(_op1, eop2->getOperand())->pullquants());
        } else {
            /* u suprotnom, moramo da preimenujemo vezanu varijablu */
            Variable var = getUniqueVariable(eop2->getOperand(), _op1);

            return make_shared<Exists>(
                    var,
                    make_shared<And>(
                        _op1,
                        eop2->getOperand()->substitute(
                            eop2->getVariable(), make_shared<VariableTerm>(var)
                        )
                    )->pullquants());
        }
    } else if(_op1->getType() == T_FORALL) {
        /* Slucaj ((forall x) A) /\ B */
        Forall * fop1 = (Forall *) _op1.get();

        /* Ako x ne postoji kao slobodna varijabla u B, tada je dovoljno
        samo izvuci kvantifikator (forall x) (A /\ B) */
        if(!_op2->containsVariable(fop1->getVariable(), true)) {
            return make_shared<Forall>(fop1->getVariable(), 
                make_shared<And>(fop1->getOperand(), _op2)->pullquants());
        } else {
            /* u suprotnom, moramo da preimenujemo vezanu varijablu */
            Variable var = getUniqueVariable(fop1->getOperand(), _op2);
            return make_shared<Forall>(var, make_shared<And>(fop1->getOperand()->
                substitute(fop1->getVariable(), make_shared<VariableTerm>(var)),
                _op2)->pullquants());
        }
    } else if(_op2->getType() == T_FORALL) {
        /* Slucaj A /\ (forall x) B */
        Forall * fop2 = (Forall *) _op2.get();

        /* Ako x ne postoji kao slobodna varijabla u A, tada je dovoljno
        samo izvuci kvantifikator (forall x) (A /\ B) */
        if(!_op1->containsVariable(fop2->getVariable(), true)) {
            return make_shared<Forall>(fop2->getVariable(), 
                make_shared<And>(_op1, fop2->getOperand())->pullquants());
        } else {
            /* u suprotnom, moramo da preimenujemo vezanu varijablu */
            Variable var = getUniqueVariable(fop2->getOperand(), _op1);

            return make_shared<Forall>(var, make_shared<And>(_op1,
                        fop2->getOperand()->substitute(fop2->getVariable(),
                        make_shared<VariableTerm>(var)))->pullquants());
        }
    } else {
        /* Formula je oblika A /\ B, gde ni A ni B nemaju kvantifikator kao vodeci
        veznik. U tom slucaju, ne radimo nista, jer su svi kvantifikatori izvuceni. */
        return shared_from_this();
    }
}



Formula Or::pullquants()
{
    /* Ako je formula oblika (exists x) A \/ (exists y) B */
    if(_op1->getType() == T_EXISTS && _op2->getType() == T_EXISTS) {
        Exists * fop1 = (Exists *) _op1.get();
        Exists * fop2 = (Exists *) _op2.get();

        /* Specijalno, ako je formula oblika (exists x) A \/ (exists x) B
        tada ne moramo da preimenujemo vezanu promenljivu, vec samo
        izvlacimo kvantifikator (exists x) (A \/ B), a zatim iz podformule
        rekurzivno izvucemo kvantifikatore */
        if(fop1->getVariable() == fop2->getVariable()) {
            return make_shared<Exists>(fop1->getVariable(), 
                make_shared<Or>(fop1->getOperand(),
                fop2->getOperand())->pullquants());
        } else {
            /* U suprotnom, uvodimo novu promenljivu koja se ne pojavljuje
            ni u jednoj od  formula A i B */
            Variable var = 
                getUniqueVariable(fop1->getOperand(), fop2->getOperand());

            return make_shared<Exists>(var, make_shared<Or>(
                fop1->getOperand()->substitute(fop1->getVariable(),
                make_shared<VariableTerm>(var)),
                fop2->getOperand()->substitute(fop2->getVariable(),
                make_shared<VariableTerm>(var)))->pullquants());
        }
    } else if(_op1->getType() == T_EXISTS) {
        /* Slucaj ((exists x) A) \/ B */
        Exists * eop1 = (Exists *) _op1.get();

        /* Ako x ne postoji kao slobodna varijabla u B, tada je dovoljno
        samo izvuci kvantifikator (exists x) (A /\ B) */
        if(!_op2->containsVariable(eop1->getVariable(), true)) {
            return make_shared<Exists>(eop1->getVariable(), 
                make_shared<Or>(eop1->getOperand(), _op2)->pullquants());
        } else {
            /* u suprotnom, moramo da preimenujemo vezanu varijablu */
            Variable var = getUniqueVariable(eop1->getOperand(), _op2);

            return make_shared<Exists>(var,
                make_shared<Or>(eop1->getOperand()->
                substitute(eop1->getVariable(),
                make_shared<VariableTerm>(var)),
                _op2)->pullquants());
        }
    } else if(_op2->getType() == T_EXISTS) {
        /* Slucaj A \/ (exists x) B */
        Exists * eop2 = (Exists *) _op2.get();

        /* Ako x ne postoji kao slobodna varijabla u A, tada je dovoljno
        samo izvuci kvantifikator (exists x) (A \/ B) */
        if(!_op1->containsVariable(eop2->getVariable(), true)) {
            return make_shared<Exists>(eop2->getVariable(), 
                make_shared<Or>(_op1, eop2->getOperand())->pullquants());
        } else {
            /* u suprotnom, moramo da preimenujemo vezanu varijablu */
            Variable var = getUniqueVariable(eop2->getOperand(), _op1);

            return make_shared<Exists>(var, make_shared<Or>(_op1,
                eop2->getOperand()->substitute(eop2->getVariable(),
                make_shared<VariableTerm>(var)))->pullquants());
        }
    } else if(_op1->getType() == T_FORALL) {
        /* Slucaj ((forall x) A) \/ B */
        Forall * fop1 = (Forall *) _op1.get();

        /* Ako x ne postoji kao slobodna varijabla u B, tada je dovoljno
        samo izvuci kvantifikator (forall x) (A \/ B) */
        if(!_op2->containsVariable(fop1->getVariable(), true)) {
            return make_shared<Forall>(fop1->getVariable(),
                make_shared<Or>(fop1->getOperand(), _op2)->pullquants());
        } else {
            /* u suprotnom, moramo da preimenujemo vezanu varijablu */
            Variable var = getUniqueVariable(fop1->getOperand(), _op2);

            return make_shared<Forall>(var, make_shared<Or>(
                fop1->getOperand()->substitute(fop1->getVariable(),
                make_shared<VariableTerm>(var)), _op2)->pullquants());
        }
    } else if(_op2->getType() == T_FORALL) {
        /* Slucaj A \/ (forall x) B */
        Forall * fop2 = (Forall *) _op2.get();

        /* Ako x ne postoji kao slobodna varijabla u A, tada je dovoljno
        samo izvuci kvantifikator (forall x) (A \/ B) */
        if(!_op1->containsVariable(fop2->getVariable(), true)) {
            return make_shared<Forall>(fop2->getVariable(),
                make_shared<Or>(_op1, fop2->getOperand())->pullquants());
        } else {
            /* u suprotnom, moramo da preimenujemo vezanu varijablu */
            Variable var = getUniqueVariable(fop2->getOperand(), _op1);

            return make_shared<Forall>(var, make_shared<Or>(_op1,
                fop2->getOperand()->substitute(fop2->getVariable(),
                make_shared<VariableTerm>(var)))->pullquants());
        }
    } else {
        /* Formula je oblika A \/ B, gde ni A ni B nemaju kvantifikator kao vodeci
        veznik. U tom slucaju, ne radimo nista, jer su svi kvantifikatori izvuceni. */
        return shared_from_this();
    }
}

Formula Imp::pullquants()
{
    /* Implikacija ne bi trebalo da se pojavi, jer je formula vec u NNF-u */
    throw "pullquants not applicable";
}

Formula Iff::pullquants()
{
    /* Ekvivalencija ne bi trebalo da se pojavi, jer je formula vec u NNF-u */
    throw "pullquants not applicable";
}

Formula Forall::pullquants()
{
    /* U slucaju Forall formule, ne radimo nista */
    return shared_from_this();
}

Formula Exists::pullquants()
{
    /* U slucaju Exists formule, ne radimo nista */
    return shared_from_this();
}


// ---------------------------------------------------------------------


// ---------------------------------------------------------------------

// Funkcije za PRENEX normalnu formu 

Formula AtomicFormula::prenex()
{
    /* U slucaju atomicke formule ne radimo nista */
    return shared_from_this(); 
}

Formula Not::prenex()
{
    /* U slucaju negacije ne radimo nista, zato sto podrazumevamo
    da je formula vec u NNF-u, pa su negacije spustene do atoma */
    return shared_from_this();
}

Formula And::prenex()
{
    /* U slucaju formule oblika A /\ B, najpre transforimisemo A i B 
    u PRENEX, a zatim iz dobijene konjukcije izvucemo kvantifikatore */
    Formula pr_op1 = _op1->prenex();
    Formula pr_op2 = _op2->prenex();

    return make_shared<And>(pr_op1, pr_op2)->pullquants();
}

Formula Or::prenex()
{
    /* U slucaju formule oblika A \/ B, najpre transforimisemo A i B 
    u PRENEX, a zatim iz dobijene disjunkcije izvucemo kvantifikatore */

    Formula pr_op1 = _op1->prenex();
    Formula pr_op2 = _op2->prenex();

    return make_shared<Or>(pr_op1, pr_op2)->pullquants();
}

Formula Imp::prenex()
{
    /* Implikacija ne bi trebalo da se pojavi, jer je formula vec u NNF-u */
    throw "Prenex not applicable";
}

Formula Iff::prenex()
{
    /* Ekvivalencija ne bi trebalo da se pojavi, jer je formula vec u NNF-u */
    throw "Prenex not applicable";
}

Formula Forall::prenex()
{
    /* U slucaju univerzalnog kvantifikatora, potrebno je samo podformulu
    svesti na prenex formu */
    return make_shared<Forall>(_v, _op->prenex());
}

Formula Exists::prenex()
{
    /* U slucaju egzistencijalnog kvantifikatora, potrebno je samo podformulu
    svesti na prenex formu */
    return make_shared<Exists>(_v, _op->prenex());
}

// -----------------------------------------------------------------------

// Funkcije za skolemizaciju ---------------------------------------------

/* Kod funkcija za skolemizaciju, podrazumeva se da je formula u PRENEX
   normalnoj formi, tj. oblika (Q1 x1) (Q2 x2) ... (Qn xn) A, gde su 
   Qi \in {forall, exists}, dok je A formula bez kvantifikatora koja ne
   sadrzi druge varijable osim x1,...,xn */

Formula BaseFormula::skolem(Signature &, vector<Variable> &&)
{
    /* Podrazumevano, za formulu bez kvantifikatora, ne radimo nista */
    return shared_from_this();
}

Formula Forall::skolem(Signature & s, vector<Variable> && vars)
{
    /* Ako je formula oblika (forall x) A, tada samo dodajemo varijablu
    x u sekvencu univerzalno kvantifikovanih varijabli, i zatim pozivamo
    rekurzivni poziv za podformulu */
    vars.push_back(_v);
    return make_shared<Forall>(_v, _op->skolem(s, std::move(vars)));
}

Formula Exists::skolem(Signature & s, vector<Variable> && vars)
{
    /* Kada naidjemo na egzistencijalni kvantifikator (exists y), 
    tada je potrebno eliminisati ga, uvodjenjem novog funkcijskog simbola 
    u signaturu, pa zato najpre uvodimo novi simbol koji nije prisutan u 
    signaturu s */
    FunctionSymbol f = getUniqueFunctionSymbol(s);

    /* Dodajemo f u signaturu sa arnoscu k = vars.size(), tj. arnost je jednaka
    broju univerzalno kvantifikovanih varijabli koje prethode egzistencijalnom
    kvantifikatoru. Primetimo da ako je k = 0, tj. ako nije bilo univerzalnih
    kvantifikatora pre datog egzistencijalnog, tada ce se uvesti u signaturu
    novi funkcijski simbol arnosti 0, tj. simbol konstante 
    (Skolemova konstanta). */
    s.addFunctionSymbol(f, vars.size());

    /* Kreiramo vektor termova x1,x2,...,xk */
    vector<Term> varTerms;

    for(unsigned i = 0; i < vars.size(); i++) {
        varTerms.push_back(make_shared<VariableTerm>(vars[i]));
    }

    /* Kreiramo term f(x1,...,xk) */
    Term t = make_shared<FunctionTerm>(s, f, varTerms);

    /* Zamenjujemo u podformuli y -> f(x1,...,xk), a zatim nastavljamo
    rekurzivno skolemizaciju u podformuli. */
    return _op->substitute(_v, t)->skolem(s, std::move(vars));
}


// -----------------------------------------------------------------------

// Konkretna interpretacija ----------------------------------------------

Zero::Zero()
  : Function(0)
{}

unsigned Zero::eval(const vector<unsigned> & args)
{
    if(args.size() > 0) {
        throw "Arguments number mismatch";
    }

    return 0;
}

One::One()
    : Function(0)
{}

unsigned One::eval(const vector<unsigned> & args)
{
    if(args.size() > 0) {
        throw "Arguments number mismatch";
    }

    return 1;
}

Plus::Plus(unsigned domain_size)
    : Function(2), _domain_size(domain_size)
{}

unsigned Plus::eval(const vector<unsigned> & args)
{
    if(args.size() != 2) {
        throw "Arguments number mismatch";
    }

    return (args[0] + args[1]) % _domain_size;
}

Times::Times(unsigned domain_size)
    : Function(2), _domain_size(domain_size)
{}

unsigned Times::eval(const vector<unsigned> & args)
{
    if(args.size() != 2) {
        throw "Arguments number mismatch";
    }

    return (args[0] * args[1]) % _domain_size;
}

Even::Even()
    : Relation(1)
{}

bool Even::eval(const vector<unsigned> & args) {
    if(args.size() != 1) {
        throw "Arguments number mismatch";
    }

    return args[0] % 2 == 0;
}

Odd::Odd()
    : Relation(1)
{}

bool Odd::eval(const vector<unsigned> & args) {
    if(args.size() != 1) {
        throw "Arguments number mismatch";
    }

    return args[0] % 2 == 1;
}

Equal::Equal()
    : Relation(2)
{}

bool Equal::eval(const vector<unsigned> & args) {
    if(args.size() != 2) {
        throw "Arguments number mismatch";
    }

    return args[0] == args[1];
}

/* Klasa predstavlja binarnu relaciju manje ili jednako */
LowerOrEqual::LowerOrEqual()
    : Relation(2)
{}

bool LowerOrEqual::eval(const vector<unsigned> & args) {
    if(args.size() != 2) {
        throw "Arguments number mismatch";
    }

    return args[0] <= args[1];
}

