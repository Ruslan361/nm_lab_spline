#pragma once

#include <vector>
#include <cmath>

class Spline
{
public:

    std::vector<double> A;
    std::vector<double> B;
    std::vector<double> C;
    std::vector<double> D;
    std::vector<double> grid;

    int size;

    int indOf(double x);

    double operator()(double x);

    // Первая производная сплайна
    double ds(double x);

    // Вторая производная сплайна
    double d2s(double x);
};

