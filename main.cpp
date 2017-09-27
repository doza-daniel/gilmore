#include "first_order_logic.h"
#include "gilmore.h"
#include "herbrand.h"

#include <memory>
#include <vector>

int main()
{
    Signature s;

    s.addFunctionSymbol("c1", 0);
    s.addFunctionSymbol("c2", 0);
    s.addPredicateSymbol("p1", 2);
    s.addFunctionSymbol("f", 2);

    Term c1 = std::make_shared<FunctionTerm>(s, "c1");
    Term c2 = std::make_shared<FunctionTerm>(s, "c2");
    Term f = std::make_shared<FunctionTerm>(s, "f", std::vector<Term>{c1, c2});
    Term x = std::make_shared<VariableTerm>("x");

    Formula a = std::make_shared<Atom>(s, "p1", std::vector<Term>{f, x});

    HerbrandUniverse hu(s, a);
    std::cout << hu << std::endl;
}
