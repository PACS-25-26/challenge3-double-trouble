#ifndef JACOBISOLVER_HPP
#define JACOBISOLVER_HPP

#include "MPIGrid.hpp"
#include <functional>

using Function = std::function<double(double, double)>;

class JacobiSolver {
private:
    MPIGrid& grid;
    Function forcing_term;

public:
    double tolerance;
    int max_iterations;
    
    // Constructor
    JacobiSolver(MPIGrid& grid, 
                 Function f,
                 double tol, 
                 int max_iter);
    
    void iterate();
    double compute_local_error() const;
    bool check_global_convergence(double local_error);
    int solve();
    double compute_l2_error(std::function<double(double, double)> exact_solution) const;
};

#endif