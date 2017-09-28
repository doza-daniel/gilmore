#include "gilmore.h"
#include "herbrand.h"

#include <sstream>

std::string gilmore(Signature s, const Formula &f)
{
    auto tf = removeUniversalQ(f->nnf()->prenex()->skolem(s));

    HerbrandUniverse universe(s, tf);
    VariableSet variables;
    tf->getVars(variables);


    std::stringstream ss;
    ss << tf << std::endl;

    return ss.str();
}
