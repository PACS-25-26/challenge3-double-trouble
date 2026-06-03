#!/bin/bash

SOLVER="./jacobi_solver"
OUTPUT_DIR="test/data"
RESULTS_FILE="$OUTPUT_DIR/results.csv"
GRID_SIZES=(16 32 64 128)
MPI_PROCS=(1 2 4)
FORCING="8*pi^2*sin(2*pi*x)*sin(2*pi*y)"
EXACT="sin(2*pi*x)*sin(2*pi*y)"

mkdir -p $OUTPUT_DIR
echo "n,solver,mpi_procs,time,iterations,l2_error" > $RESULTS_FILE

# Check solvers exist
if [ ! -f "$SOLVER" ]; then
    echo "Error: solvers not found. Run 'make' first."
    exit 1
fi

echo "Starting parallelization test"

for n in "${GRID_SIZES[@]}"; do
    for np in "${MPI_PROCS[@]}"; do
        echo "Running parallel: n=$n, MPI processes=$np"
        OUTPUT=$(mpirun -np $np $SOLVER $n "$FORCING" "$EXACT" 2>&1 | head -20)
        TIME=$(echo "$OUTPUT"  | grep "Time:"       | awk '{print $2}')
        ITERS=$(echo "$OUTPUT" | grep "Iterations:" | awk '{print $2}')
        L2=$(echo "$OUTPUT"    | grep "L2 error:"   | awk '{print $3}')
        echo "parallel,$n,$np,$TIME,$ITERS,$L2" >> $RESULTS_FILE
        echo "  → Time: ${TIME}s | Iterations: $ITERS | L2: $L2"
    done
done

echo "Done. Results saved to $RESULTS_FILE"