#ifndef HERBRAND_H
#define HERBRAND_H

#include <string>
#include <iostream>

#include "first_order_logic.h"

class HerbrandUniverse {
    std::string m_greet;
public:
    HerbrandUniverse(std::string greet);

    friend std::ostream & operator << (std::ostream & ostr, const HerbrandUniverse & hu);
};

#endif
