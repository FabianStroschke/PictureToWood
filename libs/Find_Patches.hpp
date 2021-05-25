//
// Created by fabian on 25.05.21.
//

#ifndef PICTURETOWOOD_FIND_PATCHES_HPP
#define PICTURETOWOOD_FIND_PATCHES_HPP

#include <vector>
#include "Cell.hpp"
#include <functional>

std::vector<cell *> findMatchingPatches(const std::vector<cell>& target, std::vector<cell>& source, const std::function<long(const cell &, const cell &)> &comp);

long compareGray(const cell& a, const cell& b);



#endif //PICTURETOWOOD_FIND_PATCHES_HPP
