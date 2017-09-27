#include "gilmore.h"

Gilmore::Gilmore(std::string greet)
    : m_greet(greet)
{}

std::ostream & operator << (std::ostream & ostr, const Gilmore & g) 
{
    ostr << "Greeting: " << g.m_greet;
    return ostr;
}
