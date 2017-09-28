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
        Term t = std::make_shared<FunctionTerm>(m_signature, *i, vector<Term>{});
        m_level.insert(t);
    }
}

void HerbrandUniverse::nextLevel()
{
    for (auto i = m_functions.begin(); i != m_functions.end(); i++) {
        unsigned arity;
        m_signature.checkFunctionSymbol(*i, arity);

        std::vector<Term> tmp;
        std::copy(m_level.begin(), m_level.end(), std::back_inserter(tmp));

        do {
            Term t = std::make_shared<FunctionTerm>(
                    m_signature,
                    *i,
                    std::vector<Term>(tmp.begin(), tmp.begin() + arity)
            );

            // ugly
            if (std::find_if(
                        m_level.begin(),
                        m_level.end(),
                        [t](const Term &x){ return x->equalTo(t); }) == m_level.end()) {
                m_level.insert(t);
            }
        } while (std::next_permutation(tmp.begin(), tmp.end()));
    }
}

ostream & operator << (ostream & out, const HerbrandUniverse & hu) {
    out << "Functions:" << endl;
    std::copy(
            hu.m_functions.begin(),
            hu.m_functions.end(),
            std::ostream_iterator<FunctionSymbol>(std::cout, ", ")
    );

    out << endl << "Level:" << endl;
    std::copy(
            hu.m_level.begin(),
            hu.m_level.end(),
            std::ostream_iterator<Term>(std::cout, ", ")
    );
    return out;
}


