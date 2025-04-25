#include "Functions.h"

double TestFunc(double x) {
    if (x >= -1.0 && x <= 0.0)
        return x * x * x + 3.0 * x * x;
    else if (x >= 0.0 && x <= 1.0)
        return -x * x * x + 3.0 * x * x;
}

double dTF(double x) {
    if (x >= -1.0 && x <= 0.0)
        return 3.0*x*x + 6*x;
    else if (x >= 0.0 && x <= 1.0)
        return -3.0*x * x + 3.0 * x;
}

double d2TF(double x) {
    if (x >= -1.0 && x <= 0.0)
        return 6.0 * x + 6.0;
    else if (x >= 0.0 && x <= 1.0)
        return -6.0 * x + 6.0;
}


double MainFunc1(double x) {
    return x * std::sin(x) / 3.0;
}

double dF1(double x) {
    return (std::sin(x) + x * std::cos(x)) / 3.0;
}

double d2F1(double x) {
    return (2.0 * std::cos(x) - x * std::sin(x)) / 3.0;
}


double MainFunc2(double x) {
    return std::sqrt(1.0 + x * x * x * x);
}

double dF2(double x) {
    return (2.0 * x * x * x) / (std::sqrt(1.0 + x * x * x * x));
}

double d2F2(double x) {
    return (2.0 * x * x) * (x * x * x * x + 3.0) / 
           (std::pow(1.0 + x * x * x * x, 1.5));
}

double MainFunc3(double x) {
    return std::sqrt(std::exp(x) - 1.0);
}

double dF3(double x) {
    return std::exp(x) / (2.0 * std::sqrt(std::exp(x) - 1.0));
}

double d2F3(double x) {
    return std::exp(x) / (2.0 * std::sqrt(std::exp(x) - 1.0)) - 
           std::exp(2.0 * x) / 
           (4.0 * std::pow(std::exp(x) - 1.0, 1.5));
}

double OscFunc(double x)
{
    return  x * std::sin(x) / 3.0 + std::cos(10*x);
}

double dFosc(double x)
{
    return (std::sin(x) + x * std::cos(x)) / 3.0 
            - 10.0 * std::sin(10.0 * x);
}

double d2Fosc(double x)
{
    return (2.0 * std::cos(x) - x * std::sin(x)) / 3.0 -
            100.0 * std::cos(10.0 * x);
}
