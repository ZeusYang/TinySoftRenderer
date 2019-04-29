#ifndef MATHUTILS_H
#define MATHUTILS_H

#include <cmath>
#include <iostream>

/**
 * @projectName   SoftRenderer
 * @brief         math auxiliary functions.
 * @author        YangWC
 * @date          2019-04-28
 */
namespace SoftRenderer
{

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef unsigned int uint;

const double tolerance = 1e-5f;
const double pi = static_cast<double>(3.1415926535);

inline bool equal(double a, double b)
{
    return fabs(a - b) < tolerance;
}

inline double radians(double angle)
{
    return angle * pi / 180.0f;
}

inline double angles(double radians)
{
    return radians * 180.0 / pi;
}

template<typename T>
T min(T a, T b)
{
    return a > b ? b : a;
}

template<typename T>
T max(T a, T b)
{
    return a < b ? b : a;
}

}

#endif // MATHUTILS_H
