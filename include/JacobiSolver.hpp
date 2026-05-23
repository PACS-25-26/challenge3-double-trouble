#ifndef JACOBISOLVER_HPP
#define JACOBISOLVER_HPP

#include "MPIGrid.hpp"
#include <functional>

class JacobiSolver {
private:
    MPIGrid& grid;
    std::function<double(double, double)> forcing_term;

public:
    double tolerance;
    int max_iterations;
    
    // Constructor
    JacobiSolver(MPIGrid& grid, 
                 std::function<double(double, double)> f,
                 double tol, 
                 int max_iter,
                 int rank,
                 int size);
    
    void iterate();
    double compute_local_error() const;
    bool check_global_convergence(double local_error, int iteration);
    int solve();
    
    double compute_l2_error(std::function<double(double, double)> exact_solution) const;
};

#endif