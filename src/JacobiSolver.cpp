#include "JacobiSolver.hpp"
#include <cmath>
#include <iostream>
#include <omp.h>

/**
 * @brief Constructor for Jacobi Solver
 * @param grid Reference to the MPIGrid
 * @param f Forcing term function f(x, y)
 * @param tol Convergence tolerance
 * @param max_iter Maximum number of iterations
 * @param rank MPI rank
 * @param size Total MPI processes
 */
JacobiSolver::JacobiSolver(MPIGrid& grid, 
                           Function f,
                           double tol, 
                           int max_iter)
    : grid(grid), forcing_term(f), tolerance(tol), 
      max_iterations(max_iter) {}

/**
 * @brief Perform one Jacobi iteration
 * 
 * Updates each internal node following the rule:
 * U^(k+1)(i,j) = (1/4h²) * [U^(k)(i-1,j) + U^(k)(i+1,j) + U^(k)(i,j-1) + U^(k)(i,j+1) + h²*f(i,j)]
 */
void JacobiSolver::iterate() {
    const int n = grid.n;
    const int local_rows = grid.local_rows;
    const double h = grid.h;
    const double h2 = h * h;
    const int start_row = grid.start_row;
    
    auto& u_local = grid.u_local;
    auto& u_new = grid.u_new;
    
    // Loop over local rows
    #pragma omp parallel for collapse(2)
    for (int i = 1; i <= local_rows; ++i) {
        for (int j = 1; j < n - 1; ++j) {
            int idx = i * n + j;
            
            // Global coordinates for forcing term
            int global_i = start_row + i - 1;
            double x = j * h;
            double y = global_i * h;
            
            double sum = u_local[(i - 1) * n + j]
                       + u_local[(i + 1) * n + j]
                       + u_local[i * n + (j - 1)]
                       + u_local[i * n + (j + 1)];
            
            u_new[idx] = 0.25 * (sum + h2 * forcing_term(x, y));
        }
    }
    
    // Swap old and new solutions
    std::swap(u_local, u_new);
}

/**
 * @brief Compute local error in L2 norm
 */
double JacobiSolver::compute_local_error() const {
    const int n = grid.n;
    const int local_rows = grid.local_rows;
    const double h = grid.h;
    
    const auto& u_local = grid.u_local;
    const auto& u_new = grid.u_new;
    
    double local_error_sq = 0.0;
    
    #pragma omp parallel for reduction(+:local_error_sq) collapse(2)
    for (int i = 1; i <= local_rows; ++i) {
        for (int j = 1; j < n - 1; ++j) {
            int idx = i * n + j;
            double diff = u_local[idx] - u_new[idx];
            local_error_sq += diff * diff;
        }
    }
    
    return std::sqrt(h * local_error_sq);
}

/**
 * @brief Check global convergence across all processes
 * @param local_error Local error from this process
 * @param iteration Current iteration number
 * @return true if converged globally, false otherwise
 */
bool JacobiSolver::check_global_convergence(double local_error, int iteration) {
    double global_error;
    double local_error_sq = local_error * local_error;
    double global_error_sq;
    
    // Sum of squared errors across all processes
    MPI_Allreduce(&local_error_sq, &global_error_sq, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    global_error = std::sqrt(global_error_sq);
    
    return global_error < tolerance;
}

/**
 * @brief Main solver
 * In each loop, it first exchanges common rows, then performs
 * a Jacobi iteraion and then checks convergence
 * @return Number of iterations performed
 */
int JacobiSolver::solve() {

    for (int iter = 0; iter < max_iterations; ++iter) {

        iterate();
        
        if (iter % 10 == 0) {
            double local_error = compute_local_error();
            if (check_global_convergence(local_error, iter)) {
                if (grid.rank == 0) {
                    std::cout << "Converged in " << iter << " iterations!" << std::endl;
                }
                return iter;
            }
        }
    }
    return max_iterations;
}

/**
 * @brief Compute L2 error
 * @param exact_solution Function representing the exact solution u(x, y)
 * @return Global L2 error
 */
double JacobiSolver::compute_l2_error(Function exact_solution) const {
    const int n = grid.n;
    const int local_rows = grid.local_rows;
    const double h = grid.h;
    const int start_row = grid.start_row;
    
    const auto& u_local = grid.u_local;
    
    double local_error_sq = 0.0;
    
    #pragma omp parallel for reduction(+:local_error_sq) collapse(2)
    for (int i = 1; i <= local_rows; ++i) {
        for (int j = 1; j < n - 1; ++j) {
            int global_i = start_row + i - 1;
            double x = j * h;
            double y = global_i * h;
            
            double numerical = u_local[i * n + j];
            double exact = exact_solution(x, y);
            double diff = numerical - exact;
            local_error_sq += diff * diff;
        }
    }
    
    double global_error_sq;
    MPI_Allreduce(&local_error_sq, &global_error_sq, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    
    return std::sqrt(h * global_error_sq);
}