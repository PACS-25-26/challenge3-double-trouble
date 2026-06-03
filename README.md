# Challenge 3 - Parallel Jacobi Solver for the Laplace Equation
A hybrid MPI/OpenMP C++ implementation of the Jacobi iteration method to solve the 2D Laplace equation on a unit square domain with homogeneous Dirichlet boundary conditions.

---
 
## File Structure
 
```
.
├── include/
│   ├── JacobiSerial.hpp       # Serial Jacobi solver
│   ├── JacobiSolver.hpp       # Parallel Jacobi solver (MPI + OpenMP)
│   ├── MPIGrid.hpp            # MPI grid decomposition and communication
│   └── VTKWriter.hpp          # VTK file export for ParaView
├── src/
│   ├── JacobiSerial.cpp       # Serial solver implementation
│   ├── MPIGrid.cpp            # Grid constructor, boundary exchange, gather
│   ├── JacobiSolver.cpp       # Parallel solver implementation
│   ├── VTKWriter.cpp          # VTK writer implementation
│   ├── main_serial.cpp        # Entry point for the serial solver
│   └── main.cpp               # Entry point for the parallel solver
├── test/
│   ├── ????
├── Makefile
├── README.md
└── RESULT.md ????
```
 
---
 
## Dependencies
 
- **MPI** (e.g., OpenMPI or MPICH)
- **OpenMP** (included with GCC)
- **muParser** — for parsing user-defined functions at runtime

---
 
## Build and Run
 
### Build
 
```bash
make
```
 
This produces two executables: `jacobi_parallel` and `jacobi_serial`. To clean type `make clean`.
 
### Run (parallel)
 
```bash
mpirun -np <nproc> ./jacobi_parallel <n> "<forcing_term>" ["<exact_solution>"]
```
 
- `<n>` -> grid size (n × n points)
- `<forcing_term>` -> forcing function f(x, y) as a string (supported variables: `x`, `y`; constant: `pi`)
- `<exact_solution>` -> used to compute the L2 error. This parameter is optional. If the user does not know the exact solution it can be omitted, the code will still run but the L2 error will not be computed
**Example:**
 
```bash
mpirun -np 4 ./jacobi_parallel 64 "8*pi^2*sin(2*pi*x)*sin(2*pi*y)" "sin(2*pi*x)*sin(2*pi*y)"
```
 
### Run (serial)
 
```bash
./jacobi_serial <n> "<forcing_term>" ["<exact_solution>"]
```
 
**Example:**
 
```bash
./jacobi_serial 64 "8*pi^2*sin(2*pi*x)*sin(2*pi*y)" "sin(2*pi*x)*sin(2*pi*y)"
```
 
---
 
## Implementation
 
The solver uses the Jacobi iteration method to solve the 2D Laplace equation with homogeneous Dirichlet boundary conditions. The domain is decomposed into horizontal row blocks, one per MPI process, with rows distributed as evenly as possible. Each process stores its local block plus two ghost rows, which are updated before each iteration via `MPI_Sendrecv` with neighboring ranks. Global convergence is checked every 10 iterations using `MPI_Allreduce`. The inner loops are parallelized with OpenMP (`#pragma omp parallel for collapse(2)`). Forcing terms and exact solutions are parsed at runtime from user-provided strings using the **muParser** library. After convergence, rank 0 gathers the full solution and exports it to a `.vtk` file readable by ParaView.
 
---
 
## Performance Results
 
Tests run with f(x,y) = 8π²sin(2πx)sin(2πy), exact solution u(x,y) = sin(2πx)sin(2πy).
 
| n   | Serial (s) | 1 proc (s) | 2 procs (s) | 4 procs (s) | L2 error |
|-----|-----------|------------|-------------|-------------|----------|
| 16  |           |            |             |             |          |
| 32  |           |            |             |             |          |
| 64  |           |            |             |             |          |
| 128 |           |            |             |             |          |
| 256 |           |            |             |             |          |
 
> Full discussion of results in [`RESULT.md`](RESULT.md). Hardware details in [`test/hw.info`](test/hw.info). ???? anche no??? bho