#ifndef HERBRAND_H
#define HERBRAND_H

#include <string>
#include <iostream>

class HerbrandUniverse {
    std::string m_greet;
public:
    HerbrandUniverse(std::string greet);
    friend std::ostream & operator << (std::ostream & ostr, const HerbrandUniverse & hu);
};

#endif
