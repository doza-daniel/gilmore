#include "herbrand.h"
#include <memory>
#include <algorithm>
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


