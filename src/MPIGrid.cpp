#include "MPIGrid.hpp"
#include <cmath>
#include <algorithm>
#include <cstddef>

/**
 * @brief Constructor of the MPIGrid
 * @param n Total grid size (n x n)
 * @param rank MPI rank of this process
 * @param size Total number of MPI processes
 */
MPIGrid::MPIGrid(int n, int rank, int size) 
    : n(n), rank(rank), size(size), h(1.0 / (n - 1)) {
    
    // Compute number of rows per process
    int rows_per_proc = (n - 2) / size;
    int remainder = (n - 2) % size;
    
    // Assign extra rows in order
    local_rows = rows_per_proc + (rank < remainder ? 1 : 0);
    
    // Calculate global indexes
    start_row = 1 + rank * rows_per_proc + std::min(rank, remainder);
    end_row = start_row + local_rows;
    
    // Allocate local storage
    u_local.resize((local_rows + 2) * n, 0.0);
    u_new.resize((local_rows + 2) * n, 0.0);
    
    initialize_boundary_conditions();
}

/**
 * @brief Initialize boundary conditions (here we use 
 * Dirichlet boundary conditions: u = 0 on all boundaries)
 */
void MPIGrid::initialize_boundary_conditions() {
	// Left and right boundaries
    for (int i = 0; i < local_rows + 2; ++i) {
        u_local[i * n + 0] = 0.0;
        u_local[i * n + (n - 1)] = 0.0;
        u_new[i * n + 0] = 0.0;
        u_new[i * n + (n - 1)] = 0.0;
    }
    
    // Bottom (y = 0) - only for rank 0
    if (rank == 0) {
        for (int i = 0; i < n; ++i) {
            u_local[i] = 0.0;
            u_new[i] = 0.0;
        }
    }
    
    // Top (y = 1) - only for last rank
    if (rank == size - 1) {
        int last_local_row = local_rows + 1;
        for (int i = 0; i < n; ++i) {
            u_local[last_local_row * n + i] = 0.0;
            u_new[last_local_row * n + i] = 0.0;
        }
    }
}

/**
 * @brief Exchange common rows with neighboring MPI ranks.
 * 
 * Sends the first internal row to rank - 1 and receives its last internal row.
 * Sends the last internal row to rank + 1 and receives its first internal row.
 */
void MPIGrid::exchange_boundaries() {
    MPI_Status status;

    // Exchange with rank-1
    if (rank > 0) {
        MPI_Sendrecv(
            &u_local[n], n, MPI_DOUBLE, rank - 1, 0,
            &u_local[0], n, MPI_DOUBLE, rank - 1, 1,
            MPI_COMM_WORLD, &status
        );
    }

    // Exchange with rank+1
    if (rank < size - 1) {
        MPI_Sendrecv(
            &u_local[local_rows * n], n, MPI_DOUBLE, rank + 1, 1,
            &u_local[(local_rows + 1) * n], n, MPI_DOUBLE, rank + 1, 0,
            MPI_COMM_WORLD, &status
        );
    }
}

/**
 * @brief Gather local solutions from all processes to rank 0
 * @param global_solution Output vector, to be used on rank 0
 */
void MPIGrid::gather_solution(std::vector<double>& global_solution) {
    if (rank == 0) {
        global_solution.resize(n * n

);
        // Copy rank 0's data
        for (int i = 0; i <= local_rows; ++i) {
            for (int j = 0; j < n; ++j) {
                global_solution[i * n + j] = u_local[i * n + j];
            }
        }
        
        // Receive from other ranks
        int current_row = end_row;
        for (int r = 1; r < size; ++r) {
            int recv_rows;
            MPI_Status status;
            MPI_Recv(&recv_rows, 1, MPI_INT, r, 2, MPI_COMM_WORLD, &status);
            
            // Receive data
            std::vector<double> recv_vector(recv_rows * n);
            MPI_Recv(recv_vector.data(), recv_rows * n, MPI_DOUBLE, r, 3, MPI_COMM_WORLD, &status);
            
            // Copy to global solution
            for (int i = 0; i < recv_rows; ++i) {
                for (int j = 0; j < n; ++j) {
                    global_solution[(current_row + i) * n + j] = recv_vector[i * n + j];
                }
            }
            current_row += recv_rows;
        }
    } else {
        // Send local data
        int send_rows = local_rows;
        if (rank == size - 1) {
	        send_rows++;
	      }
        
        MPI_Send(&send_rows, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
        MPI_Send(&u_local[1 * n], send_rows * n, MPI_DOUBLE, 0, 3, MPI_COMM_WORLD);
    }
}