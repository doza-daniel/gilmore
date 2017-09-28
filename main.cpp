#include "first_order_logic.h"
#include "gilmore.h"
#include "herbrand.h"

#include <memory>
#include <vector>

int main()
{
    Signature s;

    // constants
    s.addFunctionSymbol("c1", 0);
    s.addFunctionSymbol("c2", 0);

    // functions
    s.addFunctionSymbol("f", 2);
    s.addFunctionSymbol("g", 1);
    s.addFunctionSymbol("g", 1);

    // predicates
    s.addPredicateSymbol("p1", 2);
    s.addPredicateSymbol("==", 2);

    Term c1 = std::make_shared<FunctionTerm>(s, "c1");
    Term c2 = std::make_shared<FunctionTerm>(s, "c2");
    Term f = std::make_shared<FunctionTerm>(s, "f", std::vector<Term>{c1, c2});
    Term x = std::make_shared<VariableTerm>("x");

    Formula a = std::make_shared<Atom>(s, "p1", std::vector<Term>{f, x});

    Term g1 = std::make_shared<FunctionTerm>(s, "g", std::vector<Term>{c1});
    Term g2 = std::make_shared<FunctionTerm>(s, "g", std::vector<Term>{c2});
    Formula eq = std::make_shared<Atom>(s, "==", std::vector<Term>{g1, g2});

    Formula aNd = std::make_shared<And>(a, eq);

    std::cout << aNd << std::endl;

    HerbrandUniverse hu(s, a);
    std::cout << hu << std::endl;
    hu.nextLevel();
    std::cout << hu << std::endl;
    hu.nextLevel();
    std::cout << hu << std::endl;
}
