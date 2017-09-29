#include "first_order_logic.h"
#include "gilmore.h"
#include "herbrand.h"

#include <memory>
#include <vector>

int main()
{
    Signature s;
    //  H = (Ax)(Ay)(p(x,y)=>p(y,x))
    //  K = (Ax)(Ay)(Az)((p(x,y)/\p(y,z)) => p(x,z))
    //  L = (Ax)((Ey)p(x,y) => p(x,x))
    //
    //  Dokazati: (H /\ K) => L

    s.addPredicateSymbol("p", 2);
    Term x = std::make_shared<VariableTerm>("x");
    Term y = std::make_shared<VariableTerm>("y");
    Term z = std::make_shared<VariableTerm>("z");

    Formula H = std::make_shared<Forall>(
        "x",
        std::make_shared<Forall>(
            "y",
            std::make_shared<Imp>(
                std::make_shared<Atom>(s, "p", std::vector<Term>{x, y}),
                std::make_shared<Atom>(s, "p", std::vector<Term>{y, x})
            )
        )
    );

    Formula K = std::make_shared<Forall>(
        "x",
        std::make_shared<Forall>(
            "y",
            std::make_shared<Forall>(
                "z",
                std::make_shared<Imp>(
                    std::make_shared<And>(
                        std::make_shared<Atom>(s, "p", std::vector<Term>{x, y}),
                        std::make_shared<Atom>(s, "p", std::vector<Term>{y, z})
                    ),
                    std::make_shared<Atom>(s, "p", std::vector<Term>{x, z})
                )
            )
        )
    );

    Formula L = std::make_shared<Forall>(
        "x",
        std::make_shared<Forall>(
            "y",
            std::make_shared<Imp>(
                std::make_shared<Atom>(s, "p", std::vector<Term>{x, y}),
                std::make_shared<Atom>(s, "p", std::vector<Term>{x, x})
            )
        )
    );

    Formula f = std::make_shared<Imp>(std::make_shared<And>(H, K), L);

    std::cout << f << std::endl;

    prove(s, f);

    return 0;
}
