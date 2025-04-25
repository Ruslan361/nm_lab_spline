#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <cmath>

// Test function with piecewise cubic definition
double TestFunc(double x);
double dTF(double x);
double d2TF(double x);

// Main function 1: x*sin(x)/3
double MainFunc1(double x);
double dF1(double x);
double d2F1(double x);

// Main function 2: sqrt(1+x^4)
double MainFunc2(double x);
double dF2(double x);
double d2F2(double x);

// Main function 3: sqrt(exp(x)-1)
double MainFunc3(double x);
double dF3(double x);
double d2F3(double x);

// Oscillating function
double OscFunc(double x);
double dFosc(double x);
double d2Fosc(double x);

#endif // FUNCTIONS_H