#pragma once

#include "tridiagonal_matrix_algorithm.h"
#include "Problems.h"
#include "Spline.h"
#include <vector>

// Добавляем типы граничных условий
enum BORDER_MODE
{
    NATURAL, // Естественный сплайн S''=0
    DERIVATIVE_BOUNDS, // Заданные первые производные S'(a), S'(b)
    SECOND_DERIVATIVE_BOUNDS // Заданные вторые производные S''(a), S''(b)
    // Можно добавить другие типы, например, PERIODIC
};

class Solver {
    Problem problem;
    Spline spline;

    int n{}; // Number of nodes in the main grid
    BORDER_MODE border_mode{ NATURAL };
    double a{};
    double b{};
    double bound_val_a{};
    double bound_val_b{};
    double n_step{}; // Step for the main grid
    double N_step{}; // Step for the control grid (if calculated/used)

    double f_error{};
    double x_of_f_err{};
    double df_error{};
    double x_of_df_err{};
    double d2f_error{};
    double x_of_d2f_err{};

    // Новые переменные для хранения погрешностей на контрольной сетке
    double f_error_control_grid{};
    double df_error_control_grid{};
    double d2f_error_control_grid{};
    double x_of_f_err_control_grid{};
    double x_of_df_err_control_grid{};
    double x_of_d2f_err_control_grid{};

    std::vector<double> F_vector; // Function values on main grid
    std::vector<double> x_vector; // Main grid nodes
    std::vector<double> spline_vector; // Spline values on main grid
    std::vector<double> d_s; // Spline 1st derivative values on main grid
    std::vector<double> d2_s; // Spline 2nd derivative values on main grid
    // std::vector<double> f_vector; // Duplicate of F_vector? Remove if unused.
    std::vector<double> d_f; // Function 1st derivative values on main grid
    std::vector<double> d2_f; // Function 2nd derivative values on main grid

    void calcA();
    void calcB();
    void calcC();
    void calcD();


public:
    // Конструктор теперь принимает интервал и значения для гр. условий
    Solver(int _n, MODE _mode, BORDER_MODE _bm, double _a, double _b, double _bound_val_a = 0, double _bound_val_b = 0);

    void Solve();
    Spline& getSpline();

    // --- Add/Correct Getters ---
    int getNodeCount() const { return n; } // Get number of nodes
    double getGridStep() const { return n_step; } // Get main grid step

    // Геттеры для доступа к методам Problem
    double getProblemFunc(double x) const { return problem.func(x); }
    double getProblemDeriv(double x) const { return problem.deriv(x); }
    double getProblemDeriv2(double x) const { return problem.deriv2(x); }
    
    // Геттеры для получения информации об интервале
    double getIntervalStart() const { return a; }
    double getIntervalEnd() const { return b; }
    MODE getFunctionMode() const { return problem.getMode(); }

    std::vector<double>& getA();
    std::vector<double>& getB();
    std::vector<double>& getC();
    std::vector<double>& getD();

    std::vector<double>& getX_for_coef_table();
    std::vector<double>& getX();
    
    // Adding aliases for the getter methods that are called in mainwindow.cpp
    std::vector<double>& getX_nodes() { return getX(); }
    std::vector<double>& getF_nodes() { return getF(); }
    std::vector<double>& getS_nodes() { return getS(); }
    std::vector<double>& getDF_nodes() { return getDF(); } 
    std::vector<double>& getDS_nodes() { return getDS(); }
    std::vector<double>& getD2F_nodes() { return getD2F(); }
    std::vector<double>& getD2S_nodes() { return getD2S(); }

    std::vector<double>& getF(); // Use this instead of getY
    std::vector<double>& getDF(); // Use this instead of getD1F
    std::vector<double>& getD2F();

    std::vector<double>& getS();
    std::vector<double>& getDS(); // Use this instead of getD1S
    std::vector<double>& getD2S();

    double getF_ERRROR();
    double getF_ERRROR_X();
    double getDF_ERRROR(); // Use this instead of getErrorDerivative
    double getDF_ERRROR_X();
    double getD2F_ERRROR(); // Use this instead of getErrorSecondDerivative
    double getD2F_ERRROR_X();
    
    // Add aliases for error getters to match names used in mainwindow.cpp
    double getMaxErrorF() { return getF_ERRROR(); }
    double getMaxErrorDF() { return getDF_ERRROR(); }
    double getMaxErrorD2F() { return getD2F_ERRROR(); }

    double get_n_step(); // Duplicate of getGridStep? Keep one or rename. Let's keep get_n_step for now.
    double get_N_step();

    // Новый метод для вычисления погрешностей на контрольной сетке
    void calculateErrorsOnControlGrid(int N);

    // Новые геттеры для погрешностей на контрольной сетке
    double getMaxErrorF_ControlGrid() const { return f_error_control_grid; }
    double getMaxErrorDF_ControlGrid() const { return df_error_control_grid; }
    double getMaxErrorD2F_ControlGrid() const { return d2f_error_control_grid; }
    double getMaxErrorF_X_ControlGrid() const { return x_of_f_err_control_grid; }
    double getMaxErrorDF_X_ControlGrid() const { return x_of_df_err_control_grid; }
    double getMaxErrorD2F_X_ControlGrid() const { return x_of_d2f_err_control_grid; }
};
