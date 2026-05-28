#ifndef MPIGRID_HPP
#define MPIGRID_HPP

#include <vector>
#include <mpi.h>

class MPIGrid {
public:
    int n;                          // Total grid size (n x n)
    int rank;                       // MPI rank
    int size;                       // MPI size (number of processes)
    
    int local_rows;                 // Number of rows owned by this process
    int start_row;                  // Global index of first row
    int end_row;                    // Global index of last row (exclusive)
    
    std::vector<double> u_local;    // Local solution (includes ghost rows)
    std::vector<double> u_new;      // New solution for iteration
    
    double h;                       // Mesh spacing
    // Constructor
    MPIGrid(int n, int rank, int size);
    
    void initialize_boundary_conditions();
    void exchange_boundaries();
    
    // For VTK export
    void gather_solution(std::vector<double>& global_solution);
};

#endif