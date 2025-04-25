#include "../headers/Problems.h"
#include <cmath>
#include <limits>

double Problem::func(double x) const
{
    switch (mode) {
        case TEST:
            return x * x; // x^2
        case MAIN1:
            // sqrt(x²-1)/x
            if (std::abs(x) > 1.0) {
                return std::sqrt(x * x - 1.0) / x;
            } else {
                // Return NaN for values outside the domain
                return std::numeric_limits<double>::quiet_NaN();
            }
        case OSC:
            if (x != 0) {
                return std::sin(1.0 / x);
            } else {
                // Return NaN for x = 0
                return std::numeric_limits<double>::quiet_NaN();
            }
        default:
            return 0;
    }
}

double Problem::deriv(double x) const
{
    if (mode == MAIN1 && std::abs(x) <= 1.0) {
        // Domain error for sqrt(x²-1)/x
        return std::numeric_limits<double>::quiet_NaN();
    }
    
    if (mode == OSC && x == 0) {
        // Domain error for sin(1/x)
        return std::numeric_limits<double>::quiet_NaN();
    }
    
    // Analytical derivatives for known functions
    if (mode == TEST) {
        return 2 * x; // Derivative of x^2
    }
    
    if (mode == MAIN1) {
        // Derivative of sqrt(x²-1)/x
        double x2 = x * x;
        return x / (std::sqrt(x2 - 1.0) * x2) - std::sqrt(x2 - 1.0) / (x2);
    }
    
    // Use numerical differentiation for other cases
    const double h = 1e-6;
    return (func(x + h) - func(x - h)) / (2 * h);
}

double Problem::deriv2(double x) const
{
    if (mode == MAIN1 && std::abs(x) <= 1.0) {
        // Domain error for sqrt(x²-1)/x
        return std::numeric_limits<double>::quiet_NaN();
    }
    
    if (mode == OSC && x == 0) {
        // Domain error for sin(1/x)
        return std::numeric_limits<double>::quiet_NaN();
    }
    
    // Analytical second derivatives for known functions
    if (mode == TEST) {
        return 2.0; // Second derivative of x^2
    }
    
    // Use numerical differentiation for other cases
    const double h = 1e-5;
    return (func(x + h) - 2 * func(x) + func(x - h)) / (h * h);
}
