#include "first_order_logic.h"
#include "gilmore.h"
#include "herbrand.h"

#include <memory>
#include <vector>

int main()
{
    Signature s;

    // constants
    s.addFunctionSymbol("c", 0);

    // functions
    s.addFunctionSymbol("f", 1);
    s.addFunctionSymbol("g", 1);

    // predicates
    s.addPredicateSymbol("p", 2);
    s.addPredicateSymbol("q", 1);

    Term c = std::make_shared<FunctionTerm>(s, "c", std::vector<Term>{});

    Term f = std::make_shared<FunctionTerm>(s, "f", std::vector<Term>{c});
    Term g = std::make_shared<FunctionTerm>(s, "g", std::vector<Term>{c});

    Formula p = std::make_shared<Atom>(s, "p", std::vector<Term>{f, c});
    Formula q = std::make_shared<Atom>(s, "q", std::vector<Term>{g});

    Formula i = std::make_shared<And>(p, q);

    std::cout << "FORMULA:" << std::endl;
    std::cout << i << std::endl;

    HerbrandUniverse hu(s, i);
    std::cout << hu << std::endl;
    hu.nextLevel();
    std::cout << hu << std::endl;
    hu.nextLevel();
    std::cout << hu << std::endl;
}
