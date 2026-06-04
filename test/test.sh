#!/bin/bash

SOLVER="./jacobi_solver"
SOLVER_SERIAL="./jacobi_serial"
OUTPUT_DIR="test/data"
RESULTS_FILE="$OUTPUT_DIR/results.csv"
GRID_SIZES=(16 32 64 128 256)
MPI_PROCS=(2 4 8)
FORCING="8*pi^2*sin(2*pi*x)*sin(2*pi*y)"
EXACT="sin(2*pi*x)*sin(2*pi*y)"

mkdir -p "$OUTPUT_DIR"
echo "n,solver,mpi_procs,time,iterations,l2_error" > "$RESULTS_FILE"

# Check of solvers exist
if [ ! -f "$SOLVER" ] || [ ! -f "$SOLVER_SERIAL" ]; then
    echo "Error: solvers not found. Run 'make' first."
    exit 1
fi

echo "Starting test"

# Serial runs
echo ""
echo "--- Serial ---"
for n in "${GRID_SIZES[@]}"; do
    echo "Running serial: n=$n"
    OUTPUT=$("$SOLVER_SERIAL" "$n" "$FORCING" "$EXACT" 2>&1 | head -20)

    TIME=$(echo "$OUTPUT"  | grep "Time:"       | awk '{print $2}')
    ITERS=$(echo "$OUTPUT" | grep "Iterations:" | awk '{print $2}')
    L2=$(echo "$OUTPUT"    | grep "L2 error:"   | awk '{print $3}')

    if [ -z "$TIME" ]; then
        echo "  Warning: no output for n=$n (serial)"
    else
        echo "  → Time: ${TIME}s | Iterations: $ITERS | L2: $L2"
    fi

    echo "$n,serial,1,$TIME,$ITERS,$L2" >> "$RESULTS_FILE"
done

# Parallel runs (np >= 2)
echo ""
echo "--- Parallel ---"
for n in "${GRID_SIZES[@]}"; do
    for np in "${MPI_PROCS[@]}"; do
        export OMP_NUM_THREADS=$((8 / np))
        echo "Running parallel: n=$n, MPI processes=$np"
        OUTPUT=$(mpirun -np "$np" --map-by slot "$SOLVER" "$n" "$FORCING" "$EXACT" 2>&1 | head -20)

        TIME=$(echo "$OUTPUT"  | grep -m 1 "Time:"       | awk '{print $2}')
        ITERS=$(echo "$OUTPUT" | grep -m 1 "Iterations:" | awk '{print $2}')
        L2=$(echo "$OUTPUT"    | grep -m 1 "L2 error:"   | awk '{print $3}')

        if [ -z "$TIME" ]; then
            echo "  Warning: no output for n=$n, np=$np"
        else
            echo "  → Time: ${TIME}s | Iterations: $ITERS | L2: $L2"
        fi

        echo "$n,parallel,$np,$TIME,$ITERS,$L2" >> "$RESULTS_FILE"
    done
done

echo ""
echo "========================================"
echo "Done. Results saved to $RESULTS_FILE"