//
// Created by fabian on 31.05.21.
//

#include "Time_Measure.hpp"

void startTimer(){
    start = high_resolution_clock::now();
}

void endTimer(){
    end = high_resolution_clock::now();
}

void log(){
    duration<double, std::milli> ms_double = end - start;

    std::cout << ms_double.count() << "ms";
}