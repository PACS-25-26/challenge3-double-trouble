#include "JacobiSerial.hpp"
#include <cmath>
#include <iostream>

/**
 * @brief Constructor: initializes grid and allocates memory
 */
JacobiSerial::JacobiSerial(Function f, double tol, int max_iter, int n) : 
    forcing_term(f), tolerance(tol), max_iterations(max_iter), n(n), h(1.0 / (n - 1)) {
    U.resize(n * n, 0.0);
    U_new.resize(n * n, 0.0);
    initialize_boundary_conditions();
}

/**
 * @brief Initialize Dirichlet boundary conditions
 */
void JacobiSerial::initialize_boundary_conditions() {
    for (int j = 0; j < n; ++j) {
        U[0 * n + j]     = 0.0;
        U[(n-1) * n + j] = 0.0;
    }
    for (int i = 0; i < n; ++i) {
        U[i * n + 0]       = 0.0;
        U[i * n + (n - 1)] = 0.0;
    }
}

/**
 * @brief Perform one Jacobi iteration over all interior points.
 */
void JacobiSerial::iterate() {
    double h2 = h * h;

    for (int i = 1; i < n - 1; ++i) {
        for (int j = 1; j < n - 1; ++j) {
            double x = j * h;
            double y = i * h;

            double sum = U[(i-1) * n + j]
                       + U[(i+1) * n + j]
                       + U[i * n + (j-1)]
                       + U[i * n + (j+1)];

            U_new[i * n + j] = 0.25 * (sum + h2 * forcing_term(x, y));
        }
    }

    std::swap(U, U_new);
}

/**
 * @brief Compute L2 error between current and previous solution.
 */
double JacobiSerial::compute_error() const {
    double error_sq = 0.0;

    for (int i = 1; i < n - 1; ++i) {
        for (int j = 1; j < n - 1; ++j) {
            double diff = U[i * n + j] - U_new[i * n + j];
            error_sq += diff * diff;
        }
    }

    return std::sqrt(h * error_sq);
}

/**
 * @brief Main solve loop.
 * @return Number of iterations performed.
 */
int JacobiSerial::solve() {
    for (int iter = 0; iter < max_iterations; ++iter) {
        iterate();

        double error = compute_error();

        if (error < tolerance) {
            std::cout << "Converged in " << iter << " iterations!" << std::endl;
            return iter;
        }
    }

    std::cerr << "Warning: Max iterations reached without convergence!" << std::endl;
    return max_iterations;
}

/**
 * @brief Compute L2 error against exact solution.
 */
double JacobiSerial::compute_l2_error(Function exact_solution) const {
    double error_sq = 0.0;

    for (int i = 1; i < n - 1; ++i) {
        for (int j = 1; j < n - 1; ++j) {
            double x = j * h;
            double y = i * h;

            double diff = U[i * n + j] - exact_solution(x, y);
            error_sq += diff * diff;
        }
    }

    return std::sqrt(h * error_sq);
}