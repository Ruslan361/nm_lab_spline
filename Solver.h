#pragma once

#include "tridiagonal_matrix_algorithm.h"
#include "problems.h"
#include "spline.h"
#include <vector>

enum BORDER_MODE
{
    EGU, NOT_EGU, NONE_B
};

class Solver {
    Problem problem;
    Spline spline;

    int n{};
    BORDER_MODE border_mode{ NONE_B };
    double n_step{};
    double N_step{};

    double f_error{};
    double x_of_f_err{};
    double df_error{};
    double x_of_df_err{};
    double d2f_error{};
    double x_of_d2f_err{};

    std::vector<double> F_vector;
    std::vector<double> x_vector;
    std::vector<double> spline_vector;
    std::vector<double> d_s;
    std::vector<double> d2_s;
    std::vector<double> f_vector;
    std::vector<double> d_f;
    std::vector<double> d2_f;

    void calcA();
    void calcB();
    void calcC();
    void calcD();


public:

    Solver(int _n, MODE _mode, BORDER_MODE _bm);

    void Solve();
    Spline& getSpline();

    std::vector<double>& getA();
    std::vector<double>& getB();
    std::vector<double>& getC();
    std::vector<double>& getD();

    std::vector<double>& getX_for_coef_table();
    std::vector<double>& getX();

    std::vector<double>& getF();
    std::vector<double>& getDF();
    std::vector<double>& getD2F();

    std::vector<double>& getS();
    std::vector<double>& getDS();
    std::vector<double>& getD2S();

    double getF_ERRROR();
    double getF_ERRROR_X();
    double getDF_ERRROR();
    double getDF_ERRROR_X();
    double getD2F_ERRROR();
    double getD2F_ERRROR_X();

    double get_n_step();
    double get_N_step();

};