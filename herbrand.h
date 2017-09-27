#ifndef HERBRAND_H
#define HERBRAND_H

#include <string>
#include <iostream>

#include "first_order_logic.h"

class HerbrandUniverse {
    const Signature & m_signature;
    const Formula & m_formula;
    FunctionSet m_functions;
    std::set<Term> m_level;
public:
    HerbrandUniverse(const Signature & sig, const Formula & f);
    friend ostream & operator << (ostream & out, const HerbrandUniverse & hu);
};

#endif
