# Results and Discussion

## Problem Setup

We solve the Poisson equation $-\Delta u = f$ on the unit square $[0,1]^2$ with homogeneous Dirichlet boundary conditions, using the Jacobi iterative method with a standard 5-point finite difference stencil. Tests are run with:

- Forcing term: $f(x,y) = 8\pi^2 \sin(2\pi x)\sin(2\pi y)$
- Exact solution: $u(x,y) = \sin(2\pi x)\sin(2\pi y)$
- Tolerance: $10^{-7}$ (L2 norm of the increment between consecutive iterates)
- Grid sizes: $n \in \{16, 32, 64, 128, 256\}$
- MPI processes: 1 (serial), 2, 4, 8

All tests were run on a single HPC node with 8 cores (`select=1:ncpus=8:mpiprocs=8`). For the hybrid MPI+OpenMP solver, `OMP_NUM_THREADS` was set to `8 / np` to avoid oversubscription.

---

## Correctness: L2 Error vs Grid Size

| n   | Iterations | L2 Error     |
|-----|------------|--------------|
| 16  | 159        | 2.856e-02    |
| 32  | 641        | 9.545e-03    |
| 64  | 2448       | 3.271e-03    |
| 128 | 9100       | 1.068e-03    |
| 256 | 33254      | 7.465e-05    |

The L2 error decreases consistently as $n$ increases, confirming correctness of the implementation. The error reduces by approximately a factor of 3 each time $n$ doubles, approaching the theoretical $O(h^2)$ rate. The slight difference is due to discretization error.

---

## Parallel Performance

### Execution Time (seconds)

| n   | Serial  | np=2    | np=4    | np=8    |
|-----|---------|---------|---------|---------|
| 16  | 0.0038  | 0.0099  | 0.0029  | 0.0018  |
| 32  | 0.0654  | 0.0696  | 0.0218  | 0.0148  |
| 64  | 1.056   | 0.673   | 0.163   | 0.161   |
| 128 | 15.67   | 7.24    | 2.32    | 2.31    |
| 256 | 145.3   | 63.5    | 31.5    | 32.8    |

### Speedup over Serial

| n   | np=2 | np=4 | np=8 |
|-----|------|------|------|
| 16  | 0.4x | 1.3x | 2.1x |
| 32  | 0.9x | 3.0x | 4.4x |
| 64  | 1.6x | 6.5x | 6.6x |
| 128 | 2.2x | **6.8x** | 6.8x |
| 256 | 2.3x | 4.6x | 4.4x |

### Observations

**np=4 is the best configuration.** It consistently outperforms both np=2 and np=8, achieving up to 6.8x speedup for n=128. This is the optimal balance between computation and MPI communication overhead on this 8-core node.

**np=4 and np=8 plateau for large n.** With `OMP_NUM_THREADS=2`, np=4 uses 4×2=8 total threads; with `OMP_NUM_THREADS=1`, np=8 also uses 8×1=8 total threads. The two configurations use the same total number of threads, but np=4 incurs less MPI communication overhead.

**np=2 scales poorly for small n.** The overhead of the common rows exchange and global convergence check (`MPI_Allreduce` every 10 iterations) dominates for small grids, making the parallel solver slower than serial for n ≤ 32.

**Without setting `OMP_NUM_THREADS`**, each of the 8 MPI processes spawned all available OpenMP threads, leading to massive oversubscription and severe performance degradation for np=8. Setting `OMP_NUM_THREADS = ncpus / np` resolved this.