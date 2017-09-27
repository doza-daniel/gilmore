#include "first_order_logic.h"
#include "gilmore.h"
#include "herbrand.h"

int main()
{
    /* Definisemo strukturu */
    Signature s;

    /* Dodajemo funkcijske i predikatske simbole */
    s.addFunctionSymbol("0", 0);
    s.addFunctionSymbol("1", 0);
    s.addFunctionSymbol("+", 2);
    s.addFunctionSymbol("*", 2);
    s.addPredicateSymbol("even", 1);
    s.addPredicateSymbol("odd", 1);
    s.addPredicateSymbol("=", 2);
    s.addPredicateSymbol("<=", 2);


    /* Primeri termova i formula */

    Term t0 = make_shared<FunctionTerm>(s, "0");
    Term t1 = make_shared<FunctionTerm>(s, "1");

    Formula f0 = make_shared<Atom>(s, "even", vector<Term> { t0 });

    cout << f0 << endl;

    Formula f1 = make_shared<Atom>(s, "even", vector<Term> { t1 });

    cout << f1 << endl;

    Term tx = make_shared<VariableTerm>("x");
    Term ty = make_shared<VariableTerm>("y");

    Term xpy = make_shared<FunctionTerm>(s, "+", vector<Term> {tx, ty});

    Formula xeven = make_shared<Atom>(s, "even", vector<Term> { tx });

    Formula yeven = make_shared<Atom>(s, "even", vector<Term> { ty });

    Formula xpyeven = make_shared<Atom>(s, "even", vector<Term> { xpy });

    cout << xpyeven << endl;

    Formula xandy = make_shared<And>(xeven, yeven);
    Formula imp = make_shared<Imp>(xandy, xpyeven);

    Formula forall_x = make_shared<Forall>("x", imp);
    Formula forall_y = make_shared<Forall>("y", forall_x);

    cout << forall_y << endl;

    /* Semantika */

    /* Definisemo domen D = {0, 1, 2, ..., n-1 } */

    Domain domain;
    for(unsigned i = 0; i < 8; i++) {
        domain.push_back(i);
    }


    /* Definisemo L-strukturu */
    Structure st(s, domain);

    /* Dodeljujemo interpretacije simbolima iz signature */
    st.addFunction("0", new Zero());
    st.addFunction("1", new One());
    st.addFunction("+", new Plus(domain.size()));
    st.addFunction("*", new Times(domain.size()));
    st.addRelation("even", new Even());
    st.addRelation("odd", new Odd());
    st.addRelation("=", new Equal());
    st.addRelation("<=", new LowerOrEqual());

    /* Definisemo valuaciju */
    Valuation val(domain);
    val.setValue("x", 1);
    val.setValue("y", 3);

    /* Primeri izracunavanja vrednosti termova i formula */
    cout << xpy->eval(st, val) << endl;

    cout << forall_y->eval(st,val) << endl;

    Formula and_f = make_shared<And>(xeven, forall_y);

    cout << and_f << endl;

    cout << and_f->eval(st, val) << endl;

    Gilmore g("hello gilmore");
    cout << g << endl;

    HerbrandUniverse h("hello herbrand");
    cout << h << endl;

    return 0;
}
