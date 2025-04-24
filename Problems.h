#pragma once

#include <functional>
#include <utility>

#include "Functions.h"

enum MODE
{
    NONE, TEST, MAIN1, Main2, Main3, OSC
};



struct Problem {

    MODE mode{NONE};
   
    double a{};
    double b{};
    double mu1{};
    double mu2{};

    explicit Problem();
    explicit Problem( MODE _mode);
    void operator=(const Problem& _problem);

    double f(double x);
    double df(double x);
    double d2f(double x);
};