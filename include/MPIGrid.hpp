#ifndef MPIGRID_HPP
#define MPIGRID_HPP

#include <vector>
#include <mpi.h>

class MPIGrid {
private:
    int n_;                          // Total grid size (n x n)
    int rank_;                       // MPI rank
    int size_;                       // MPI size (number of processes)
    
    int local_rows_;                 // Number of rows owned by this process
    int start_row_;                  // Global index of first row
    int end_row_;                    // Global index of last row (exclusive)
    
    std::vector<double> u_local_;    // Local solution (includes ghost rows)
    std::vector<double> u_new_;      // New solution for iteration
    
    double h_;                       // Mesh spacing

public:
    MPIGrid(int n, int rank, int size);
    
    void initialize_boundary_conditions();
    void exchange_boundaries();
    
    // Getters
    int get_n() const { return n_; }
    int get_local_rows() const { return local_rows_; }
    int get_start_row() const { return start_row_; }
    int get_end_row() const { return end_row_; }
    double get_h() const { return h_; }
    
    std::vector<double>& get_u_local() { return u_local_; }
    std::vector<double>& get_u_new() { return u_new_; }
    const std::vector<double>& get_u_local() const { return u_local_; }
    const std::vector<double>& get_u_new() const { return u_new_; }
    
    // For VTK export
    void gather_solution(std::vector<double>& global_solution);
};

#endif