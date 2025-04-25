#include "../headers/Spline.h"
#include <algorithm>
#include <cmath>

int Spline::indOf(double x)
{
    // Обработка граничных случаев
    if (x <= grid[0]) return 0;
    // Исправлено: проверка на grid[size - 1] должна возвращать последний интервал size - 2
    if (x >= grid[size - 1]) return size - 2; 
    
    // Бинарный поиск для нахождения интервала
    int left = 0;
    int right = size - 1;
    
    while (left + 1 < right) {
        int mid = (left + right) / 2;
        if (grid[mid] <= x) {
            left = mid;
        } else {
            right = mid;
        }
    }
    
    return left;
}

double Spline::operator()(double x)
{
    int i = indOf(x);
    // Добавим проверку на случай, если indOf вернул некорректный индекс (хотя не должно)
    if (i < 0 || i >= size - 1) {
         // Попробуем вернуть значение на ближайшей границе или NaN
         if (x <= grid[0]) i = 0;
         else if (x >= grid[size - 1]) i = size - 2;
         else return std::numeric_limits<double>::quiet_NaN(); // Неожиданная ситуация
    }

    double dx = x - grid[i];
    
    // Используем C[i]/2.0 как квадратичный коэффициент
    return A[i] + B[i] * dx + (C[i] / 2.0) * (dx * dx) + D[i] * (dx * dx * dx);
}

double Spline::ds(double x)
{
    int i = indOf(x);
     // Добавим проверку на случай, если indOf вернул некорректный индекс
    if (i < 0 || i >= size - 1) {
         if (x <= grid[0]) i = 0;
         else if (x >= grid[size - 1]) i = size - 2;
         else return std::numeric_limits<double>::quiet_NaN();
    }

    double dx = x - grid[i];
    
    // Производная от (C[i]/2.0)*dx^2 равна C[i]*dx
    return B[i] + C[i] * dx + 3.0 * D[i] * dx * dx;
}

double Spline::d2s(double x)
{
    int i = indOf(x);
     // Добавим проверку на случай, если indOf вернул некорректный индекс
    if (i < 0 || i >= size - 1) {
         if (x <= grid[0]) i = 0;
         else if (x >= grid[size - 1]) i = size - 2;
         else return std::numeric_limits<double>::quiet_NaN();
    }
    
    double dx = x - grid[i];
    
    // Вторая производная от (C[i]/2.0)*dx^2 равна C[i]
    // Вторая производная от D[i]*dx^3 равна 6*D[i]*dx
    return C[i] + 6.0 * D[i] * dx;
}


