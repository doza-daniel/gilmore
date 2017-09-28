#ifndef FIRST_ORDER_LOGIC_H
#define FIRST_ORDER_LOGIC_H

#include <iostream>
#include <set>
#include <map>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <memory>
#include <functional>

using namespace std;

/* Funkcijski i predikatski simboli */
typedef string FunctionSymbol;
typedef string PredicateSymbol;

/* Signatura se sastoji iz funkcijskih i predikatskih simbola kojima
   su pridruzene arnosti (nenegativni celi brojevi) */
class Signature {
private:
    map<FunctionSymbol,  unsigned> _functions;
    map<PredicateSymbol, unsigned> _predicates;
    vector<FunctionSymbol> _uniqueConstants;
    size_t _currUniq;
public:
    Signature();

    FunctionSymbol getNewUniqueConstant();

    /* Dodavanje funkcijskog simbola date arnosti */
    void addFunctionSymbol(const FunctionSymbol & f, unsigned arity);

    /* Dodavanje predikatskog simbola date arnosti */
    void addPredicateSymbol(const PredicateSymbol & p, unsigned arity);

    /* Provera da li postoji dati funkcijski simbol, i koja mu je arnost */
    bool checkFunctionSymbol(const FunctionSymbol & f, unsigned & arity) const;

    /* Provera da li postoji dati predikatski simbol, i koja mu je arnost */
    bool checkPredicateSymbol(const PredicateSymbol & f, unsigned & arity) const;
};


/* Tip podatka za predstavljanje varijable */
typedef string Variable;

/* Skup varijabli */
typedef set<Variable> VariableSet;
typedef set<FunctionSymbol> ConstantSet;
typedef set<FunctionSymbol> FunctionSet;


class Structure; // L-strukture (videti dole)
class Valuation; // Valuacija (videti dole)

class BaseTerm;
typedef std::shared_ptr<BaseTerm> Term;

/* Apstraktna klasa BaseTerm koja predstavlja termove */
class BaseTerm : public enable_shared_from_this<BaseTerm> {

public:
    /* Termovi mogu biti ili varijable ili funkcijski simboli primenjeni
    na (0 ili vise) termova */
    enum Type { TT_VARIABLE, TT_FUNCTION };

    /* Vraca tip terma */
    virtual Type getType() const = 0;

    /* Prikazuje term */
    virtual void printTerm(ostream & ostr) const = 0;

    /* Ispituje sintaksnu jednakost termova */
    virtual bool equalTo(const Term & t) const = 0;

    /* Vraca skup svih varijabli koje se pojavljuju u termu */
    virtual void getVars(VariableSet & vars) const = 0;

    /* Vraca skup svih konstanti (funkcija arnosti 0) koje se pojavljuju u termu */
    virtual void getConstants(ConstantSet & cts) const = 0;

    /* Vraca skup svih konstanti (funkcija arnosti 0) koje se pojavljuju u termu */
    virtual void getFunctions(FunctionSet & fs) const = 0;

    /* Odredjuje da li se data varijabla nalazi u termu */
    bool containsVariable(const Variable & v) const;

    /* Odredjuje interpretaciju terma u datoj L-strukturi i datoj valuaciji */
    virtual unsigned eval(const Structure & st, const Valuation & val) const = 0;

    /* Zamena varijable v termom t */
    virtual Term substitute(const Variable & v, const Term & t) = 0;

    virtual ~BaseTerm() {}
};

ostream & operator << (ostream & ostr, const Term & t);

/* Term koji predstavlja jednu varijablu */
class VariableTerm : public BaseTerm {
private:
    Variable _v;
public:
    VariableTerm(const Variable & v);
    virtual Type getType() const;
    const Variable & getVariable() const;
    virtual void printTerm(ostream & ostr) const;
    virtual bool equalTo(const Term & t) const;
    virtual void getVars(VariableSet & vars) const;
    virtual void getConstants(ConstantSet & cts) const;
    virtual void getFunctions(FunctionSet & fs) const;
    virtual unsigned eval(const Structure & st, const Valuation & val) const;
    virtual Term substitute(const Variable & v, const Term & t);
};

/* Term koji predstavlja funkcijski simbol primenjen na odgovarajuci
   broj podtermova */
class FunctionTerm : public BaseTerm {
private:
    const Signature & _sig;
    FunctionSymbol _f;
    vector<Term> _ops;

public:
    FunctionTerm(const Signature & s, const FunctionSymbol & f, const vector<Term> & ops);
    FunctionTerm(const Signature & s, const FunctionSymbol & f, vector<Term> && ops = vector<Term> ());
    virtual Type getType() const;
    const Signature & getSignature() const;
    const FunctionSymbol & getSymbol() const;
    const vector<Term> & getOperands() const;
    virtual void printTerm(ostream & ostr) const;
    virtual bool equalTo(const Term & t) const;
    virtual void getVars(VariableSet & vars) const;
    virtual void getConstants(ConstantSet & cts) const;
    virtual void getFunctions(FunctionSet & fs) const;
    virtual unsigned eval(const Structure & st, const Valuation & val) const;
    virtual Term substitute(const Variable & v, const Term & t);
};


class BaseFormula;
typedef std::shared_ptr<BaseFormula> Formula;


// koristimo za dnf
typedef vector<Formula> LiteralList;
typedef vector<LiteralList> LiteralListList;

LiteralListList makePairs(const LiteralListList & c1, const LiteralListList & c2);

/* Apstraktna klasa kojom se predstavljaju formule */
class BaseFormula : public enable_shared_from_this<BaseFormula> {
public:

    /* Tipovi formula (dodatak u odnosu na iskaznu logiku su formule
    kod kojih je vodeci simbol univerzalni ili egzistencijalni
    kvantifikator */
    enum Type { T_TRUE, T_FALSE, T_ATOM, T_NOT,
        T_AND, T_OR, T_IMP, T_IFF, T_FORALL, T_EXISTS };

    /* Prikaz formule */
    virtual void printFormula(ostream & ostr) const = 0;

    /* Tip formule */
    virtual Type getType() const = 0;

    /* Slozenost formule */
    virtual unsigned complexity() const = 0;

    /* Sintaksna jednakost dve formule */
    virtual bool equalTo(const Formula & f) const = 0;

    /* Ocitava sve varijable koje se pojavljuju u formuli. Ako
    je zadat drugi parametar sa vrednoscu true, tada se izdvajaju samo
    slobodne varijable u formuli */
    virtual void getVars(VariableSet & vars, bool free = false) const = 0;
    virtual void getConstants(ConstantSet & cts) const = 0;
    virtual void getFunctions(FunctionSet & fs) const = 0;

    /* Ispituje da li se varijabla pojavljuje u formuli (kao slobodna ili
    vezana) */
    bool containsVariable(const Variable & v, bool free = false) const;

    /* Izracunava interpretaciju formule za datu L-strukturu i valuaciju */
    virtual bool eval(const Structure & st, const Valuation & val) const = 0;

    /* Zamena slobodnih pojavljivanja varijable v termom t */
    virtual Formula substitute(const Variable & v, const Term & t) = 0;

    /* Funkcija simplifikuje formulu (uklanja konstante i nepotrebne 
    kvantifikatore) */
    virtual Formula simplify() = 0;

    /* Funkcija svodi formulu na NNF */
    virtual Formula nnf() = 0;

    /* Pomocna funkcija za izdvajanje kvantifikatora */
    virtual Formula pullquants() = 0;

    /* Funkcija svodi formulu na PRENEX */
    virtual Formula prenex() = 0;

    /* Funkcija za skolemizaciju */
    virtual Formula skolem(Signature & s, vector<Variable> && vars = vector<Variable>());

    virtual LiteralListList listDNF() = 0;
    virtual ~BaseFormula() {}
};

ostream & operator << (ostream & ostr, const Formula & f);

/* Funkcija vraca novu varijablu koja se ne pojavljuje ni u e1 ni u e2 */
template <typename T1, typename T2>
Variable getUniqueVariable(const T1 & e1, const T2 & e2);

/* Funkcija uklanja univerzalne kvantifikatore sa pocetka izraza */
Formula removeUniversalQ(const Formula & f);

/* Funkcija vraca novi funkcijski simbol koji se ne pojavljuje u formuli */
FunctionSymbol getUniqueFunctionSymbol(const Signature & s);

/* Klasa predstavlja sve atomicke formule (True, False i Atom) */
class AtomicFormula : public BaseFormula {
public:
    virtual unsigned complexity() const;
    virtual Formula simplify();
    virtual Formula nnf();
    virtual Formula pullquants();
    virtual Formula prenex();
};

/* Klasa predstavlja logicke konstante (True i False) */
class LogicConstant : public AtomicFormula {
public:
    virtual bool equalTo(const Formula & f) const;
    virtual void getVars(VariableSet & vars, bool free) const;
    virtual void getConstants(ConstantSet & cts) const;
    virtual void getFunctions(FunctionSet & fs) const;
    virtual Formula substitute(const Variable & v, const Term & t);
};

/* Klasa predstavlja True logicku konstantu */
class True : public LogicConstant {

public:
    virtual void printFormula(ostream & ostr) const;
    virtual Type getType() const;
    virtual bool eval(const Structure & st, const Valuation & val) const;
    virtual LiteralListList listDNF();
};

/* Klasa predstavlja logicku konstantu False */
class False : public LogicConstant {

public:
    virtual void printFormula(ostream & ostr) const;
    virtual Type getType() const;
    virtual bool eval(const Structure & st, const Valuation & val) const;
    virtual LiteralListList listDNF();
};

/* Klasa predstavlja atom, koji za razliku od iskazne logike ovde ima
   znatno slozeniju strukturu. Svaki atom je predikatski simbol primenjen
   na odgovarajuci broj podtermova */
class Atom : public AtomicFormula {
private:
    const Signature & _sig;
    PredicateSymbol _p;
    vector<Term> _ops;

public:
    Atom(const Signature & s, const PredicateSymbol & p, const vector<Term> & ops);
    Atom(const Signature & s, const PredicateSymbol & p, vector<Term> && ops = vector<Term>());
    const PredicateSymbol & getSymbol() const;
    const Signature & getSignature() const;
    const vector<Term> & getOperands() const;
    virtual void printFormula(ostream & ostr) const;
    virtual Type getType() const;
    virtual bool equalTo(const Formula & f) const;

    virtual void getVars(VariableSet & vars, bool free) const;
    virtual void getConstants(ConstantSet & vars) const;
    virtual void getFunctions(FunctionSet & vars) const;
    virtual bool eval(const Structure & st, const Valuation & val) const;
    virtual Formula substitute(const Variable & v, const Term & t);
    virtual LiteralListList listDNF();
};


/* Klasa unarni veznik (obuhvata negaciju) */
class UnaryConnective : public BaseFormula {
protected:
    Formula _op;
public:
    UnaryConnective(const Formula & op);
    const Formula & getOperand() const;
    virtual unsigned complexity() const;
    virtual bool equalTo(const Formula & f) const;
    virtual void getVars(VariableSet & vars, bool free) const;
    virtual void getConstants(ConstantSet & cts) const;
    virtual void getFunctions(FunctionSet & vars) const;
};

/* Klasa koja predstavlja negaciju */
class Not : public UnaryConnective {
public:
    using UnaryConnective::UnaryConnective;
    virtual void printFormula(ostream & ostr) const;
    virtual Type getType() const;
    virtual bool eval(const Structure & st, const Valuation & val) const;
    virtual Formula substitute(const Variable & v, const Term & t);

    virtual Formula simplify();
    virtual Formula nnf();
    virtual Formula pullquants();
    virtual Formula prenex();
    virtual LiteralListList listDNF();
};

/* Klasa predstavlja sve binarne veznike */
class BinaryConnective : public BaseFormula {
protected:
    Formula _op1, _op2;
public:
    BinaryConnective(const Formula & op1, const Formula & op2);
    const Formula & getOperand1() const;
    const Formula & getOperand2() const;
    virtual unsigned complexity() const;
    virtual bool equalTo(const Formula & f) const;
    virtual void getVars(VariableSet & vars, bool free) const;
    virtual void getConstants(ConstantSet & vars) const;
    virtual void getFunctions(FunctionSet & vars) const;
};

/* Klasa predstavlja konjunkciju */
class And : public BinaryConnective {
public:
    using BinaryConnective::BinaryConnective;
    virtual void printFormula(ostream & ostr) const;
    virtual Type getType() const;
    virtual bool eval(const Structure & st, const Valuation & val) const;
    virtual Formula substitute(const Variable & v, const Term & t);

    virtual Formula simplify();
    virtual Formula nnf();
    virtual Formula pullquants();
    virtual Formula prenex();
    virtual LiteralListList listDNF();
 };


/* Klasa predstavlja disjunkciju */
class Or : public BinaryConnective {
public:
    using BinaryConnective::BinaryConnective;
    virtual void printFormula(ostream & ostr) const;
    virtual Type getType() const;
    virtual bool eval(const Structure & st, const Valuation & val) const;
    virtual Formula substitute(const Variable & v, const Term & t);

    virtual Formula simplify();
    virtual Formula nnf();
    virtual Formula pullquants();
    virtual Formula prenex();
    virtual LiteralListList listDNF();
};

/* Klasa predstavlja implikaciju */
class Imp : public BinaryConnective {
public:
    using BinaryConnective::BinaryConnective;
    virtual void printFormula(ostream & ostr) const;
    virtual Type getType() const;
    virtual bool eval(const Structure & st, const Valuation & val) const;
    virtual Formula substitute(const Variable & v, const Term & t);

    virtual Formula simplify();
    virtual Formula nnf();
    virtual Formula pullquants();
    virtual Formula prenex();
    virtual LiteralListList listDNF();
};


/* Klasa predstavlja ekvivalenciju */
class Iff : public BinaryConnective {
public:
    using BinaryConnective::BinaryConnective;
    virtual void printFormula(ostream & ostr) const;
    virtual Type getType() const;
    virtual bool eval(const Structure & st, const Valuation & val) const;
    virtual Formula substitute(const Variable & v, const Term & t);

    virtual Formula simplify();
    virtual Formula nnf();
    virtual Formula pullquants();
    virtual Formula prenex();
    virtual LiteralListList listDNF();
};

/* Klasa predstavlja kvantifikovane formule */
class Quantifier : public BaseFormula {
protected:
    Variable _v;
    Formula _op;

public:
    Quantifier(const Variable & v, const Formula & op);
    const Variable & getVariable() const;
    const Formula & getOperand() const;
    virtual unsigned complexity() const;
    virtual bool equalTo(const Formula & f) const;
    virtual void getVars(VariableSet & vars, bool free) const;
    virtual void getConstants(ConstantSet & cts) const;
    virtual void getFunctions(FunctionSet & vars) const;
    // ne moze
    virtual LiteralListList listDNF();
};

/* Klasa predstavlja univerzalno kvantifikovanu formulu */
class Forall : public Quantifier {
public:
    using Quantifier::Quantifier;
    virtual Type getType() const;
    virtual void printFormula(ostream & ostr) const;
    virtual bool eval(const Structure & st, const Valuation & val) const;
    virtual Formula substitute(const Variable & v, const Term & t);

    virtual Formula simplify();
    virtual Formula nnf();
    virtual Formula pullquants();
    virtual Formula prenex();
    virtual Formula skolem(Signature & s, vector<Variable> && vars);
};


/* Klasa predstavlja egzistencijalnog kvantifikatora */
class Exists : public Quantifier {
public:
    using Quantifier::Quantifier;
    virtual Type getType() const;
    virtual void printFormula(ostream & ostr) const;
    virtual bool eval(const Structure & st, const Valuation & val) const;
    virtual Formula substitute(const Variable & v, const Term & t);

    virtual Formula simplify();
    virtual Formula nnf();
    virtual Formula pullquants();
    virtual Formula prenex();
    virtual Formula skolem(Signature & s, vector<Variable> && vars);
};

/* Tip podataka kojim se predstavlja domen. U opstem slucaju, domen u logici
   prvog reda moze biti i beskonacan skup. Medjutim, mi cemo se u nasoj
   implementaciji zadrzati na konacnim skupovima, kako bismo mogli da
   simuliramo izracunavanje interpretacija kvantifikovanih formula. Zato
   ce nam domen uvek biti neki konacan podskup skupa prirodnih brojeva */
typedef std::vector<unsigned> Domain;

/* Apstraktni tip podatka kojim se predstavlja funkcija D^n --> D kojom
   se mogu interpretirati funkcijski simboli arnosti n */
class Function {
private:
    unsigned _arity;
public:
    Function(unsigned arity);
    unsigned getArity();
    virtual unsigned eval(const vector<unsigned> & args = vector<unsigned>()) = 0;
    virtual ~Function() {}
};


/* Apstraktni tip podataka kojim se predstavlja relacija D^n --> {0,1} kojom
   se mogu interpretirati predikatski simboli arnosti n */
class Relation {
private:
    unsigned _arity;
public:
    Relation(unsigned arity);
    unsigned getArity();
    virtual bool eval(const vector<unsigned> & args = vector<unsigned>()) = 0;
    virtual ~Relation() {}
};

/* Klasa koja predstavlja L-strukturu (ili model) nad signaturom L i domenom
   D. Model svakom funkcijskom simbolu pridruzuje funkciju odgovarajuce
   arnosti, dok svakom predikatskom simbolu pridruzuje relaciju odgovarajuce
   arnosti. */
class Structure {
private:
    const Signature & _sig;
    const Domain & _domain;
    map<FunctionSymbol, Function *> _funs;
    map<PredicateSymbol, Relation *> _rels;

public:
    Structure(const Signature & sig, const Domain & domain);

    const Signature & getSignature() const;
    const Domain & getDomain() const;

    /* Dodavanje interpretacije funkcijskom simbolu */
    void addFunction(const FunctionSymbol & fs, Function * f);

    /* Citanje interpretacije datog funkcijskog simbola */
    Function * getFunction(const FunctionSymbol & f) const;

    /* Dodavanje interpretacije predikatskom simbolu */
    void addRelation(const PredicateSymbol & ps, Relation * r);

    /* Citanje interpretacije datog predikatskog simbola */
    Relation * getRelation(const PredicateSymbol & p) const;

    ~Structure();
};

/* Klasa kojom se predstavlja valuacija. Valuacijom (nad datim domenom) se
   svakoj varijabli dodeljuje neka vrednost iz domena. Drugim recima,
   valuacija odredjuje interpretaciju varijabli. */
class Valuation {
private:
    const Domain & _domain;
    map<Variable, unsigned> _values;
public:
    Valuation(const Domain & dom);
    const Domain & getDomain() const;

    /* Postavljanje vrednosti date varijable na datu vrednost */
    void setValue(const Variable & v, unsigned value);

    /* Ocitavanje vrednosti date varijable */
    unsigned getValue(const Variable & v) const;
};

// Konkretna interpretacija
/* Klasa predstavlja funkciju-konstantu 0 */
class Zero : public Function {
public:
    Zero();
    virtual unsigned eval(const vector<unsigned> & args);
};

/* Klasa predstavlja funkciju-konstantu 1 */
class One : public Function {
public:
    One();
    virtual unsigned eval(const vector<unsigned> & args);
};

/* Klasa predstavlja binarnu funkciju sabiranja po modulu n */
class Plus : public Function {
private:
    unsigned _domain_size;
public:
    Plus(unsigned domain_size);
    virtual unsigned eval(const vector<unsigned> & args);
};

/* Klasa predstavlja binarnu funkciju mnozenja po modulu n */
class Times : public Function {
private:
    unsigned _domain_size;
public:
    Times(unsigned domain_size);
    virtual unsigned eval(const vector<unsigned> & args);
};

/* Klasa predstavlja unarnu relaciju koja sadrzi sve parne brojeve */
class Even : public Relation {
public:
    Even();
    virtual bool eval(const vector<unsigned> & args);
};

/* Klasa predstavlja unarnu relaciju koja sadrzi sve neparne brojeve */
class Odd : public Relation {

public:
    Odd();
    virtual bool eval(const vector<unsigned> & args);
};

/* Klasa predstavlja binarnu relaciju jednakosti */
class Equal : public Relation {
public:
    Equal();
    virtual bool eval(const vector<unsigned> & args);
};

/* Klasa predstavlja binarnu relaciju manje ili jednako */
class LowerOrEqual : public Relation {
public:
    LowerOrEqual();
    virtual bool eval(const vector<unsigned> & args);
};



#endif
