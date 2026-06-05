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
│   ├── test.sh                # Automated parallelization test script
|   ├── RESULTS.md
│   └── data/
│       └── results.csv        # Test output
├── Makefile
└── README.md
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
 
This produces two executables: `jacobi_solver` (parallel) and `jacobi_serial` (serial).
 
### Run (parallel)
 
```bash
mpirun -np <nproc> ./jacobi_solver <n> "<forcing_term>" ["<exact_solution>"]
```
 
- `<n>` — grid size (n × n points)
- `<forcing_term>` — forcing function f(x, y) as a string (supported variables: `x`, `y`; constant: `pi`)
- `<exact_solution>` — used to compute the L2 error. Optional: if omitted the solver still runs but the L2 error is not reported

**Example:**
 
```bash
mpirun -np 4 ./jacobi_solver 64 "8*pi^2*sin(2*pi*x)*sin(2*pi*y)" "sin(2*pi*x)*sin(2*pi*y)"
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

## Reproducing Results

The test script runs the serial solver and the parallel solver with np = 2, 4, 8 across grid sizes n = 16, 32, 64, 128, 256 and saves results to `test/data/results.csv`.

On an HPC cluster with PBS:

```bash
qsub -I -q cpu -l select=1:ncpus=8:mpiprocs=8
make
bash test/test.sh
```

---
 
## Implementation
 
The solver uses the Jacobi iteration method to solve the 2D Laplace equation with homogeneous Dirichlet boundary conditions. The domain is decomposed into horizontal row blocks, one per MPI process, with rows distributed as evenly as possible. Each process stores its local block plus two ghost rows, which are updated before each iteration via `MPI_Sendrecv` with neighboring ranks. Global convergence is checked every 10 iterations using `MPI_Allreduce`. The inner loops are parallelized with OpenMP (`#pragma omp parallel for collapse(2)`). Forcing terms and exact solutions are parsed at runtime from user-provided strings using the **muParser** library. After convergence, rank 0 gathers the full solution and exports it to a `.vtk` file readable by ParaView.
 
---
 
## Performance Results
 
Tests run on a single node (8 cores) with f(x,y) = 8π²sin(2πx)sin(2πy), exact solution u(x,y) = sin(2πx)sin(2πy), tolerance 1e-7.

| n   | Serial (s) | np=2 (s) | np=4 (s) | np=8 (s) | L2 error  |
|-----|------------|----------|----------|----------|-----------|
| 16  | 0.0038     | 0.0099   | 0.0029   | 0.0018   | 2.856e-02 |
| 32  | 0.0654     | 0.0696   | 0.0218   | 0.0148   | 9.545e-03 |
| 64  | 1.056      | 0.673    | 0.163    | 0.161    | 3.271e-03 |
| 128 | 15.67      | 7.24     | 2.32     | 2.31     | 1.068e-03 |
| 256 | 145.3      | 63.5     | 31.5     | 32.8     | 7.465e-05 |
 
> Full discussion of results in [`RESULT.md`](RESULT.md).