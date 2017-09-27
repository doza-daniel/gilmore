#include "herbrand.h"

HerbrandUniverse::HerbrandUniverse(std::string greet)
    : m_greet(greet)
{}

std::ostream & operator << (std::ostream & ostr, const HerbrandUniverse & hu)
{
    ostr << "Greeting: " << hu.m_greet;
    return ostr;
}

