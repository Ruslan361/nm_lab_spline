#pragma once

#include <vector>

// Tridiagonal matrix algorithm (Thomas algorithm)
// Solves a system of linear equations with a tridiagonal matrix
// a - lower diagonal, b - main diagonal, c - upper diagonal, d - right side
std::vector<double> tridiagonal_matrix_algorithm(
    const std::vector<double>& a, 
    const std::vector<double>& b, 
    const std::vector<double>& c, 
    const std::vector<double>& d, 
    int n);
