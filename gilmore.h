#ifndef GILMORE_H
#define GILMORE_H

#include <string>
#include <iostream>

class Gilmore {
    std::string m_greet;
public:
    Gilmore(std::string greet = "default");
    friend std::ostream & operator << (std::ostream & ostr, const Gilmore & g);
};

#endif
