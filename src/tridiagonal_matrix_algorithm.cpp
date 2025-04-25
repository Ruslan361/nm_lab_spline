#include "../headers/tridiagonal_matrix_algorithm.h"
#include <stdexcept>
#include <cmath>

std::vector<double> tridiagonal_matrix_algorithm(
    const std::vector<double>& a, 
    const std::vector<double>& b, 
    const std::vector<double>& c, 
    const std::vector<double>& d, 
    int n) 
{
    // Check sizes
    if (a.size() < n || b.size() < n || c.size() < n || d.size() < n) {
        throw std::logic_error("Wrong size of vectors!");
    }
    
    std::vector<double> alpha(n);
    std::vector<double> beta(n);
    std::vector<double> x(n);
    
    // Forward sweep
    alpha[0] = c[0] / b[0];
    beta[0] = d[0] / b[0];
    
    for (int i = 1; i < n; i++) {
        double denominator = b[i] - a[i] * alpha[i-1];
        if (std::abs(denominator) < 1e-10) {
            throw std::runtime_error("Error in tridiagonal matrix algorithm: division by zero");
        }
        alpha[i] = c[i] / denominator;
        beta[i] = (d[i] - a[i] * beta[i-1]) / denominator;
    }
    
    // Backward sweep
    x[n-1] = beta[n-1];
    
    for (int i = n-2; i >= 0; i--) {
        x[i] = beta[i] - alpha[i] * x[i+1];
    }
    
    return x;
}
