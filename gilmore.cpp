#include "gilmore.h"
#include "herbrand.h"

#include <algorithm>

const size_t MAX_ITERATIONS = 5;

void prove(Signature s, const Formula &f)
{
    gilmore(s, std::make_shared<Not>(f));
}

void gilmore(Signature s, const Formula &f)
{
    auto tf = removeUniversalQ(f->nnf()->prenex()->skolem(s));

    HerbrandUniverse universe(s, tf);
    VariableSet v;
    tf->getVars(v);
    std::vector<Variable> variables;

    std::copy(v.begin(), v.end(), std::back_inserter(variables));

    size_t nvars = variables.size();

    size_t iteration;
    for (iteration = 0; iteration < MAX_ITERATIONS; ++iteration) {
        std::vector< std::vector<Term> > substitutions =
            product(universe.level(), nvars);

        Formula sub = tf;

        for (size_t i = 0; i < nvars; ++i) {
            sub = sub->substitute(variables[i], substitutions[0][i]);
        }

        for (size_t i = 1; i < substitutions.size(); ++i) {
            Formula nsub = tf;
            for (size_t j = 0; j < nvars; ++j) {
                nsub = nsub->substitute(variables[j], substitutions[i][j]);
            }

            sub = std::make_shared<And>(sub, nsub);
        }

        LiteralListList dnf = sub->listDNF();

        size_t removeCount = 0;

        for (LiteralList conjuncts : dnf) {
            LiteralList positive;
            LiteralList negative;
            bool found = false;

            for (auto literal : conjuncts) {
                if (literal->getType() == BaseFormula::T_NOT) {
                    Formula t = ((Not*)literal.get())->getOperand();
                    if (std::find_if(positive.begin(), positive.end(),
                                [t](const Formula &x) { return x->equalTo(t); }) != positive.end()) {
                        found = true;
                        removeCount += 1;
                        break;
                    } else {
                        negative.push_back(t);
                    }
                } else if (literal->getType() == BaseFormula::T_ATOM) {
                    if (std::find_if(negative.begin(), negative.end(),
                                [literal](const Formula &x) { return x->equalTo(literal); }) != negative.end()) {
                        found = true;
                        removeCount += 1;
                        break;
                    } else {
                        positive.push_back(literal);
                    }
                } else {
                    throw "Bad type";
                }
            }

            if (!found) {
                std::cout << "Conjunctions not removed: [";
                for (auto literal : conjuncts) {
                    std::cout << literal << ", ";
                }
                std::cout << "]" << std::endl;
            }
        }

        if (removeCount == dnf.size()) {
            std::cout << "UNSAT" << std::endl;
            break;
        } else {
            std::cout << "Next level..." << std::endl;
        }

        universe.nextLevel();
    }
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
