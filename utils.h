#ifndef UTILS_H
#define UTILS_H

#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <random>
#include <cstdlib>


const double Infinity = std::numeric_limits<double>::infinity();
const double Pi = 3.141592653589793285;

inline double degrees_to_radian(double degrees){
    return degrees * Pi / 180.0;
}

inline double random_double_pseudo() {
    // Returns a random real in [0,1).
    return std::rand() / (RAND_MAX + 1.0);
}

inline double random_double() {
    static std::uniform_real_distribution<double> distribution(0.0, 1.0);
    static std::mt19937 generator;
    return distribution(generator);
}

inline double random_double(double min, double max) {
    // Returns a random real in [min,max).
    return min + (max-min)*random_double();
}


#include "color.h"
#include "interval.h"
#include "ray.h"
#include "vec3.h"

#endif