#include "../headers/Problems.h"
#include "../headers/Functions.h" // Include the header with the new functions
#include <cmath>
#include <limits>

double Problem::func(double x) const
{
    switch (mode) {
        case TEST:
            return TestFunc(x);
        case MAIN1:
            return MainFunc1(x);
        case MAIN2:
            return MainFunc2(x);
        case MAIN3:
            return MainFunc3(x);
        case OSC:
            return OscFunc(x);
        default:
            return 0;
    }
}

double Problem::deriv(double x) const
{
    switch (mode) {
        case TEST:
            return dTF(x);
        case MAIN1:
            return dF1(x);
        case MAIN2:
            return dF2(x);
        case MAIN3:
            return dF3(x);
        case OSC:
            return dFosc(x);
        default:
            // Use numerical differentiation as a fallback if needed, or return NaN
            // const double h = 1e-6;
            // return (func(x + h) - func(x - h)) / (2 * h);
            return std::numeric_limits<double>::quiet_NaN();
    }
}

double Problem::deriv2(double x) const
{
    switch (mode) {
        case TEST:
            return d2TF(x);
        case MAIN1:
            return d2F1(x);
        case MAIN2:
            return d2F2(x);
        case MAIN3:
            return d2F3(x);
        case OSC:
            return d2Fosc(x);
        default:
            // Use numerical differentiation as a fallback if needed, or return NaN
            // const double h = 1e-5;
            // return (func(x + h) - 2 * func(x) + func(x - h)) / (h * h);
            return std::numeric_limits<double>::quiet_NaN();
    }
}
