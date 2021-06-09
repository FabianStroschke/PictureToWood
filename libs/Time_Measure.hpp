//
// Created by fabian on 31.05.21.
//

#ifndef PICTURETOWOOD_TIME_MEASURE_HPP
#define PICTURETOWOOD_TIME_MEASURE_HPP

#include <chrono>
#include <iostream>


using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::duration;
using std::chrono::milliseconds;

auto static start = high_resolution_clock::now();
auto static end = high_resolution_clock::now();

void startTimer();
void endTimer();
double log();


#endif //PICTURETOWOOD_TIME_MEASURE_HPP
