#include "herbrand.h"
#include <memory>
#include <algorithm>
#include <vector>
#include <iterator>
#include <iostream>

HerbrandUniverse::HerbrandUniverse(const Signature & sig,const Formula & f)
    : m_signature(sig), m_formula(f)
{
    m_formula->getFunctions(m_functions);
    ConstantSet cs;
    m_formula->getConstants(cs);

    for (auto i = cs.begin(); i != cs.end(); i++) {
        Term t = std::make_shared<FunctionTerm>(m_signature, *i, std::vector<Term>{});
        m_level.insert(t);
    }
    if (m_level.size() == 0) {
        std::cout << "No constants found, inserting unique..." << std::endl;
        m_level.insert(
            std::make_shared<FunctionTerm>(m_signature, m_signature.getNewUniqueConstant(), std::vector<Term>{})
        );
    }
}

std::set<Term> HerbrandUniverse::level() const
{
    return m_level;
}

void HerbrandUniverse::nextLevel()
{
    std::vector<Term> tmp;
    std::copy(m_level.begin(), m_level.end(), std::back_inserter(tmp));

    for (auto i = m_functions.begin(); i != m_functions.end(); i++) {
        unsigned arity;
        m_signature.checkFunctionSymbol(*i, arity);

        do {
            std::vector<Term> operands;
            if (arity > tmp.size()) {
                operands = std::vector<Term>(tmp.begin(), tmp.end());
                for (size_t i = tmp.size(); i < arity; i++) {
                    operands.push_back(
                        std::make_shared<FunctionTerm>(
                            m_signature, m_signature.getNewUniqueConstant(), std::vector<Term>{}
                        )
                    );
                }
            } else {
                operands = std::vector<Term>(tmp.begin(), tmp.begin() + arity);
            }
            Term application = std::make_shared<FunctionTerm>(
                    m_signature,
                    *i,
                    operands
            );

            if (std::find_if(
                        m_level.begin(),
                        m_level.end(),
                        [application](const Term &x){ return x->equalTo(application); }) == m_level.end()) {
                m_level.insert(application);
            }
        } while (std::next_permutation(tmp.begin(), tmp.end()));
    }
}

ostream & operator << (ostream & out, const HerbrandUniverse & hu) {
    out << "Functions:" << endl;
    out << "{ ";
    std::copy(
            hu.m_functions.begin(),
            hu.m_functions.end(),
            std::ostream_iterator<FunctionSymbol>(std::cout, ", ")
    );
    out << " }";

    out << endl << "Level:" << endl;
    out << "{ ";
    std::copy(
            hu.m_level.begin(),
            hu.m_level.end(),
            std::ostream_iterator<Term>(std::cout, ", ")
    );
    out << " }";
    return out;
}


