#include "../headers/Solver.h"
#include "../headers/tridiagonal_matrix_algorithm.h"
#include <cmath>
#include <stdexcept>
#include "../headers/Solver.h"
#include <algorithm>
#include <limits> // Для std::numeric_limits

// ... существующий код tridiagonal_matrix_algorithm ...

// Конструктор Solver
Solver::Solver(int _n, MODE _mode, BORDER_MODE _bm, double _a, double _b, double _bound_val_a, double _bound_val_b)
    : n(_n),
      border_mode(_bm),
      a(_a), // Сохраняем начало интервала
      b(_b), // Сохраняем конец интервала
      bound_val_a(_bound_val_a), // Сохраняем значение для левой границы
      bound_val_b(_bound_val_b), // Сохраняем значение для правой границы
      problem(_mode) // Инициализируем problem
{
    // Проверка корректности интервала
    if (b <= a) {
        throw std::invalid_argument("Некорректный интервал: b должно быть больше a.");
    }
    if (n < 4) {
         throw std::invalid_argument("Количество узлов должно быть не менее 4 для кубического сплайна.");
    }

    // Вычисляем шаг сетки
    n_step = (b - a) / (n - 1);

    // Генерируем узлы сетки в интервале [a, b]
    x_vector.resize(n);
    for (int i = 0; i < n; i++) {
        x_vector[i] = a + i * n_step;
    }

    // Вычисляем значения функции, первой и второй производной в узлах
    F_vector.resize(n);
    d_f.resize(n);
    d2_f.resize(n);

    int valid_points_count = 0;
    for (int i = 0; i < n; i++) {
        double x = x_vector[i];
        F_vector[i] = problem.func(x); // Используем problem, инициализированный ранее
        if (!std::isnan(F_vector[i])) {
             d_f[i] = problem.deriv(x);
             d2_f[i] = problem.deriv2(x);
             valid_points_count++;
        } else {
             d_f[i] = std::numeric_limits<double>::quiet_NaN();
             d2_f[i] = std::numeric_limits<double>::quiet_NaN();
        }
    }

    // Проверка на достаточность точек после вычисления значений
    if (valid_points_count < 4) {
         throw std::runtime_error("Недостаточно валидных точек функции в заданном интервале.");
    }
    // Инициализация N_step (если используется контрольная сетка)
    // N_step = ...; // Логика для N_step, если она нужна
}

void Solver::Solve()
{
    // Фильтруем точки, где функция не определена (NaN)
    std::vector<double> valid_x;
    std::vector<double> valid_F;

    for (size_t i = 0; i < x_vector.size(); i++) {
        if (!std::isnan(F_vector[i])) {
            valid_x.push_back(x_vector[i]);
            valid_F.push_back(F_vector[i]);
        }
    }

    // Строим сплайн только если есть достаточно точек (минимум 2 для гр.условий 1-го рода, 4 для естественного)
    int min_points = (border_mode == NATURAL) ? 4 : 2;
    if (valid_x.size() < min_points) {
         throw std::runtime_error("Недостаточно валидных точек для построения сплайна с выбранными гр. условиями.");
    }

    // Сохраняем оригинальные векторы
    std::vector<double> original_x_vector = x_vector;
    std::vector<double> original_F_vector = F_vector;
    std::vector<double> original_d_f = d_f;
    std::vector<double> original_d2_f = d2_f;


    // Подменяем векторы на валидные для вычисления коэффициентов
    x_vector = valid_x;
    F_vector = valid_F;
    // Важно: d_f и d2_f не используются напрямую в calc*, но могут понадобиться для других гр. условий

    // Вычисляем коэффициенты сплайна
    calcC(); // Использует подмененные x_vector, F_vector
    calcA(); // Использует подмененный F_vector
    calcB(); // Использует подмененные x_vector, F_vector и spline.C
    calcD(); // Использует подмененный x_vector и spline.C

    // Восстанавливаем оригинальные векторы
    x_vector = original_x_vector;
    F_vector = original_F_vector;
    d_f = original_d_f;
    d2_f = original_d2_f;

    // Устанавливаем *оригинальную* сетку в объект сплайна для последующих вычислений
    spline.grid = x_vector;
    spline.size = x_vector.size(); // Размер сплайна теперь соответствует оригинальной сетке

    // --- Вычисление значений сплайна и ошибок на оригинальной сетке ---
    spline_vector.resize(x_vector.size());
    d_s.resize(x_vector.size());
    d2_s.resize(x_vector.size());

    // Диапазон, где сплайн был фактически построен
    double spline_min_x = valid_x.front();
    double spline_max_x = valid_x.back();

    for (int i = 0; i < x_vector.size(); i++) {
        double x = x_vector[i];
        // Вычисляем сплайн только если точка x находится в диапазоне,
        // где сплайн был построен (между первой и последней валидной точкой)
        if (x >= spline_min_x && x <= spline_max_x) {
             spline_vector[i] = spline(x); // spline() использует коэффициенты, вычисленные по valid точкам
             d_s[i] = spline.ds(x);
             d2_s[i] = spline.d2s(x);
        } else {
            // Точка вне диапазона построенного сплайна
            spline_vector[i] = std::numeric_limits<double>::quiet_NaN();
            d_s[i] = std::numeric_limits<double>::quiet_NaN();
            d2_s[i] = std::numeric_limits<double>::quiet_NaN();
        }
         // Дополнительно обнуляем сплайн там, где исходная функция была NaN
         if (std::isnan(F_vector[i])) {
             spline_vector[i] = std::numeric_limits<double>::quiet_NaN();
             d_s[i] = std::numeric_limits<double>::quiet_NaN();
             d2_s[i] = std::numeric_limits<double>::quiet_NaN();
         }
    }

    // Вычисляем ошибки интерполяции только для валидных точек в диапазоне сплайна
    f_error = 0;
    df_error = 0;
    d2f_error = 0;
    x_of_f_err = std::numeric_limits<double>::quiet_NaN();
    x_of_df_err = std::numeric_limits<double>::quiet_NaN();
    x_of_d2f_err = std::numeric_limits<double>::quiet_NaN();

    // Отдельные флаги для инициализации каждого типа ошибки
    bool first_valid_f_error = true;
    bool first_valid_df_error = true;
    bool first_valid_d2f_error = true;

    for (int i = 0; i < x_vector.size(); i++) {
        double x_val = x_vector[i];
        
        // Обрабатываем погрешность функции
        if (!std::isnan(F_vector[i]) && !std::isnan(spline_vector[i])) {
            double f_diff = std::abs(F_vector[i] - spline_vector[i]);
            
            if (first_valid_f_error) {
                f_error = f_diff;
                x_of_f_err = x_val;
                first_valid_f_error = false;
            } else if (f_diff > f_error) {
                f_error = f_diff;
                x_of_f_err = x_val;
            }
        }
        
        // Обрабатываем погрешность первой производной
        if (!std::isnan(d_f[i]) && !std::isnan(d_s[i])) {
            double df_diff = std::abs(d_f[i] - d_s[i]);
            
            if (first_valid_df_error) {
                df_error = df_diff;
                x_of_df_err = x_val;
                first_valid_df_error = false;
            } else if (df_diff > df_error) {
                df_error = df_diff;
                x_of_df_err = x_val;
            }
        }
        
        // Обрабатываем погрешность второй производной
        if (!std::isnan(d2_f[i]) && !std::isnan(d2_s[i])) {
            double d2f_diff = std::abs(d2_f[i] - d2_s[i]);
            
            if (first_valid_d2f_error) {
                d2f_error = d2f_diff;
                x_of_d2f_err = x_val;
                first_valid_d2f_error = false;
            } else if (d2f_diff > d2f_error) {
                d2f_error = d2f_diff;
                x_of_d2f_err = x_val;
            }
        }
    }
}

// --- Implementations for Coefficient Calculations ---

void Solver::calcA() {
    // Implementation needed: Calculate spline.A coefficients
    // spline.A[i] = F_vector[i]; (Usually)
    spline.A.resize(x_vector.size() - 1); // A has size n-1
    for (size_t i = 0; i < x_vector.size() - 1; ++i) {
        spline.A[i] = F_vector[i];
    }
}

void Solver::calcC() {
    // Implementation needed: Calculate spline.C coefficients using Tridiagonal Matrix Algorithm
    int N = x_vector.size(); // Number of points (use size of valid points)
    if (N < 2) return; // Need at least 2 points
    spline.C.resize(N);
    std::vector<double> h(N - 1);
    for (int i = 0; i < N - 1; ++i) {
        h[i] = x_vector[i+1] - x_vector[i];
        if (h[i] <= 0) throw std::runtime_error("Invalid grid step in calcC");
    }

    // Setup tridiagonal system (Ax = F)
    std::vector<double> lower_diag(N);
    std::vector<double> main_diag(N);
    std::vector<double> upper_diag(N);
    std::vector<double> F(N);

    // Fill system based on border conditions
    if (border_mode == NATURAL) {
        if (N < 4) throw std::runtime_error("Natural spline requires at least 4 points.");
        // Natural boundary conditions: S''(a) = 0, S''(b) = 0 => C[0] = 0, C[N-1] = 0
        main_diag[0] = 1.0;
        upper_diag[0] = 0.0;
        F[0] = 0.0;

        for (int i = 1; i < N - 1; ++i) {
            lower_diag[i] = h[i-1];
            main_diag[i] = 2.0 * (h[i-1] + h[i]);
            upper_diag[i] = h[i];
            F[i] = 3.0 * ((F_vector[i+1] - F_vector[i]) / h[i] - (F_vector[i] - F_vector[i-1]) / h[i-1]);
        }

        lower_diag[N-1] = 0.0;
        main_diag[N-1] = 1.0;
        F[N-1] = 0.0;
    } else { // First derivative boundary conditions
         if (N < 2) throw std::runtime_error("First derivative spline requires at least 2 points.");
        // S'(a) = bound_val_a, S'(b) = bound_val_b
        main_diag[0] = 2.0 * h[0];
        upper_diag[0] = h[0];
        F[0] = 3.0 * ((F_vector[1] - F_vector[0]) / h[0] - bound_val_a);

        for (int i = 1; i < N - 1; ++i) {
            lower_diag[i] = h[i-1];
            main_diag[i] = 2.0 * (h[i-1] + h[i]);
            upper_diag[i] = h[i];
            F[i] = 3.0 * ((F_vector[i+1] - F_vector[i]) / h[i] - (F_vector[i] - F_vector[i-1]) / h[i-1]);
        }

        lower_diag[N-1] = h[N-2];
        main_diag[N-1] = 2.0 * h[N-2];
        F[N-1] = 3.0 * (bound_val_b - (F_vector[N-1] - F_vector[N-2]) / h[N-2]);
    }

    // Solve the system for C coefficients
    spline.C = tridiagonal_matrix_algorithm(lower_diag, main_diag, upper_diag, F, N); // Pass N as the size argument
}

void Solver::calcB() {
    // Implementation needed: Calculate spline.B coefficients
    int N = x_vector.size();
    if (N < 2) return;
    spline.B.resize(N - 1); // B has size n-1
    std::vector<double> h(N - 1);
     for (int i = 0; i < N - 1; ++i) {
         h[i] = x_vector[i+1] - x_vector[i];
         if (h[i] <= 0) throw std::runtime_error("Invalid grid step in calcB");
     }

    for (int i = 0; i < N - 1; ++i) {
        // Corrected formula for B[i]
        spline.B[i] = (F_vector[i+1] - F_vector[i]) / h[i] - h[i] * (2.0 * spline.C[i] + spline.C[i+1]) / 6.0;
    }
}

void Solver::calcD() {
    // Implementation needed: Calculate spline.D coefficients
    int N = x_vector.size();
    if (N < 2) return;
    spline.D.resize(N - 1); // D has size n-1
    std::vector<double> h(N - 1);
     for (int i = 0; i < N - 1; ++i) {
         h[i] = x_vector[i+1] - x_vector[i];
         if (h[i] <= 0) throw std::runtime_error("Invalid grid step in calcD");
     }

    for (int i = 0; i < N - 1; ++i) {
        // Corrected formula for D[i]
        spline.D[i] = (spline.C[i+1] - spline.C[i]) / (6.0 * h[i]);
    }
}

// --- Implementations for Getters ---

// int Solver::getNodeCount() const { return n; } // Already defined inline in .h
// double Solver::getGridStep() const { return n_step; } // Already defined inline in .h

Spline& Solver::getSpline()
{
    return spline;
}

std::vector<double>& Solver::getA()
{
    return spline.A;
}

std::vector<double>& Solver::getB()
{
    return spline.B;
}

std::vector<double>& Solver::getC()
{
    return spline.C;
}

std::vector<double>& Solver::getD()
{
    return spline.D;
}

std::vector<double>& Solver::getX_for_coef_table()
{
    // Return the grid used when coefficients were calculated (valid_x)
    // This requires storing valid_x or returning a copy.
    // For simplicity now, return the original grid. Revisit if needed.
    return x_vector;
}

std::vector<double>& Solver::getX()
{
    return x_vector;
}

std::vector<double>& Solver::getF()
{
    return F_vector;
}

std::vector<double>& Solver::getDF()
{
    return d_f;
}

std::vector<double>& Solver::getD2F()
{
    return d2_f;
}

std::vector<double>& Solver::getS()
{
    return spline_vector;
}

std::vector<double>& Solver::getDS()
{
    return d_s;
}

std::vector<double>& Solver::getD2S()
{
    return d2_s;
}

double Solver::getF_ERRROR()
{
    return f_error;
}

double Solver::getF_ERRROR_X()
{
    return x_of_f_err;
}

double Solver::getDF_ERRROR()
{
    return df_error;
}

double Solver::getDF_ERRROR_X()
{
    return x_of_df_err;
}

double Solver::getD2F_ERRROR()
{
    return d2f_error;
}

double Solver::getD2F_ERRROR_X()
{
    return x_of_d2f_err;
}

double Solver::get_n_step()
{
    return n_step;
}

double Solver::get_N_step()
{
    // Calculate if needed, or return stored value if Solver calculates it
    // Example calculation (if N is known):
    // if (n > 1 && N > 1 && b > a) {
    //     return (b - a) / (N - 1); // N needs to be passed or stored
    // }
    return N_step; // Return stored value for now
}

void Solver::calculateErrorsOnControlGrid(int N) {
    if (N < 2 || a >= b) return;
    
    // Инициализируем переменные для хранения погрешностей
    f_error_control_grid = 0.0;
    df_error_control_grid = 0.0;
    d2f_error_control_grid = 0.0;
    x_of_f_err_control_grid = std::numeric_limits<double>::quiet_NaN();
    x_of_df_err_control_grid = std::numeric_limits<double>::quiet_NaN();
    x_of_d2f_err_control_grid = std::numeric_limits<double>::quiet_NaN();
    
    // Создаем контрольную сетку
    double control_step = (b - a) / (N - 1);
    N_step = control_step; // Сохраняем шаг контрольной сетки
    
    // Флаги для первой инициализации погрешностей
    bool first_valid_f_error = true;
    bool first_valid_df_error = true;
    bool first_valid_d2f_error = true;
    
    // Проходим по всем точкам контрольной сетки
    for (int i = 0; i < N; ++i) {
        double x_val = (i == 0) ? a : ((i == N - 1) ? b : a + i * control_step);
        
        // Вычисляем значения функции и сплайна в точке x_val
        double func_val = getProblemFunc(x_val);
        double spl_val = getSpline()(x_val);
        
        // Вычисляем значения первых производных
        double func_deriv_val = getProblemDeriv(x_val);
        double spl_deriv_val = getSpline().ds(x_val);
        
        // Вычисляем значения вторых производных
        double func_deriv2_val = getProblemDeriv2(x_val);
        double spl_deriv2_val = getSpline().d2s(x_val);
        
        // Вычисляем погрешность функции
        if (!std::isnan(func_val) && !std::isnan(spl_val)) {
            double f_diff = std::abs(func_val - spl_val);
            
            if (first_valid_f_error) {
                f_error_control_grid = f_diff;
                x_of_f_err_control_grid = x_val;
                first_valid_f_error = false;
            } else if (f_diff > f_error_control_grid) {
                f_error_control_grid = f_diff;
                x_of_f_err_control_grid = x_val;
            }
        }
        
        // Вычисляем погрешность первой производной
        if (!std::isnan(func_deriv_val) && !std::isnan(spl_deriv_val)) {
            double df_diff = std::abs(func_deriv_val - spl_deriv_val);
            
            if (first_valid_df_error) {
                df_error_control_grid = df_diff;
                x_of_df_err_control_grid = x_val;
                first_valid_df_error = false;
            } else if (df_diff > df_error_control_grid) {
                df_error_control_grid = df_diff;
                x_of_df_err_control_grid = x_val;
            }
        }
        
        // Вычисляем погрешность второй производной
        if (!std::isnan(func_deriv2_val) && !std::isnan(spl_deriv2_val)) {
            double d2f_diff = std::abs(func_deriv2_val - spl_deriv2_val);
            
            if (first_valid_d2f_error) {
                d2f_error_control_grid = d2f_diff;
                x_of_d2f_err_control_grid = x_val;
                first_valid_d2f_error = false;
            } else if (d2f_diff > d2f_error_control_grid) {
                d2f_error_control_grid = d2f_diff;
                x_of_d2f_err_control_grid = x_val;
            }
        }
    }
}


