#include "first_order_logic.h"


// Definicije funkcija clanica -----------------------------------------

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

void LogicConstant::getVars(VariableSet &, bool) const
{
    return;
}

void LogicConstant::getConstants(ConstantSet &) const
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

void UnaryConnective::getVars(VariableSet & vars, bool free) const
{
    _op->getVars(vars, free);
}

void UnaryConnective::getConstants(ConstantSet & cts) const
{
    _op->getConstants(cts);
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

Variable getUniqueVariable(const Formula & f, const Term & t)
{
    static unsigned i = 0;

    Variable v;

    do {
        v = string("uv") + to_string(++i);
    } while(t->containsVariable(v) || f->containsVariable(v));

    return v;
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

