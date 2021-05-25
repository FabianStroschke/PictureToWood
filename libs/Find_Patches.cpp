//
// Created by fabian on 25.05.21.
//

#include <iostream>
#include "Find_Patches.hpp"


std::vector<cell *> findMatchingPatches(const std::vector<cell>& target, std::vector<cell>& source, const std::function<long(const cell &, const cell &)> &comp){
    std::vector<cell *> match;
    for(const cell& t : target){
        cell *best = &(source.front());
        long min = comp(t, *best);
        for(cell& s : source){
            long diff = comp(t,s);
            if(diff < min){
                min = diff;
                best = &s;
            }
        }
        match.push_back(best);
    }
    return match;
}

long compareGray(const cell& a, const cell& b){
    int numPixel = a.img_gray.dataend - a.img_gray.datastart;
    long sum = 0;
    for(int i = 0; i < numPixel; i++){
        sum += std::abs((a.img_gray.data[i] - b.img_gray.data[i]));
    }
    return sum;
}

