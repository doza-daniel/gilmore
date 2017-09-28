#include "gilmore.h"
#include "herbrand.h"

#include <sstream>

std::string gilmore(Signature s, const Formula &f)
{
    auto tf = removeUniversalQ(f->nnf()->prenex()->skolem(s));

    HerbrandUniverse universe(s, tf);
    VariableSet variables;
    tf->getVars(variables);


    return "UNSAT";
}

std::vector< std::vector<Term> > product(std::set<Term> s, int repeat)
{
    std::vector< std::vector<Term> > res;
    variations(0, std::vector<Term>(repeat), s, res);
    return res;
}

void variations(size_t pos, std::vector<Term> currPerm, std::set<Term> items, std::vector< std::vector<Term> >& res)
{
    if (pos == currPerm.size()) {
        res.push_back(currPerm);
    } else {
        for (auto i = items.begin(); i != items.end(); ++i) {
            currPerm.at(pos) = *i;
            variations(pos + 1, currPerm, items, res);
        }
    }
}
