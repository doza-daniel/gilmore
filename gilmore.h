#ifndef GILMORE_H
#define GILMORE_H

#include <string>
#include <vector>
#include <set>

#include "first_order_logic.h"

std::string gilmore(Signature s, const Formula &f);


std::vector< std::vector<Term> > product(std::set<Term> s, int repeat = 2);

void variations(size_t pos, std::vector<Term> currPerm, std::set<Term> items, std::vector< std::vector<Term> >& res);
#endif
