#include "MPIGrid.hpp"
#include "JacobiSolver.hpp"
#include "VTKWriter.hpp"
#include <mpi.h>
#include <iostream>
#include <cmath>
#include <muParser.h>
#include <memory>

/**
 * @brief Forcing term wrapper, parsed from string
 * To be chosen by the user
 */
class ForcingTermParser {
private:
    std::string expr;
    
public:
    ForcingTermParser(const std::string& expr) : expr(expr) {}
    
    double operator()(double x, double y) const {
        thread_local mu::Parser parser;
        thread_local double x_val = 0.0;
        thread_local double y_val = 0.0;
        thread_local bool initialized = false;
    
        if (!initialized) {
            parser.DefineVar("x", &x_val);
            parser.DefineVar("y", &y_val);
            parser.DefineConst("pi", M_PI);
            parser.SetExpr(expr);
            initialized = true;
        }
    
        x_val = x;
        y_val = y;
        return parser.Eval();
    }
};

/**
 * @brief Exact solution wrapper
 * The user can specify the exact solution to compute the L2 error (not mandatory)
 */
class ExactSolutionParser {
private:
    std::string expr;
    bool is_defined;
    
public:
    ExactSolutionParser() : is_defined(false) {}
    
    ExactSolutionParser(const std::string& expr) : expr(expr), is_defined(true) {}
    
    double operator()(double x, double y) const {
        if (!is_defined) {
		        return 0.0;
		    }
		    
        thread_local mu::Parser parser;
        thread_local double x_val = 0.0;
        thread_local double y_val = 0.0;
        thread_local bool initialized = false;
			
        if (!initialized) {
            parser.DefineVar("x", &x_val);
            parser.DefineVar("y", &y_val);
            parser.DefineConst("pi", M_PI);
            parser.SetExpr(expr);
            initialized = true;
        }
			
        x_val = x;
        y_val = y;
        return parser.Eval();
    }
    
    bool defined() const { return is_defined; }
};

int main(int argc, char** argv) {
    // Initialize MPI
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Fixed parameters
    const double tolerance = 1e-7;
    const int max_iter = 1e6;

    // Parse command line arguments
		if (argc < 3) {
		    if (rank == 0) {
		        std::cerr << "Wrong number of arguments! Right usage is: mpirun -np <nproc> " << argv[0] 
		                  << " <n> \"<forcing_term>\" [\"<exact_solution>\"]" << std::endl;
		    }
		    MPI_Finalize();
		    return 1;
		}

    int n = std::atoi(argv[1]);
    if (n < 2) {
        if (rank == 0)
            std::cerr << "Error: n must be >= 2" << std::endl;
        MPI_Finalize();
        return 1;
    }
    std::string forcing_expr = argv[2];
    std::string exact_expr = (argc > 3) ? argv[3] : "";

    // Create parsers
    std::shared_ptr<ForcingTermParser> forcing_parser;
    std::shared_ptr<ExactSolutionParser> exact_parser;

    try {
        forcing_parser = std::make_shared<ForcingTermParser>(forcing_expr);
        
        if (!exact_expr.empty()) {
            exact_parser = std::make_shared<ExactSolutionParser>(exact_expr);
        } else {
            exact_parser = std::make_shared<ExactSolutionParser>();
        }
        
    } catch (mu::Parser::exception_type &e) {
        if (rank == 0) {
            std::cerr << "Parser error: " << e.GetMsg() << std::endl;
        }
        MPI_Finalize();
        return 1;
    }

    // Create lambda wrappers for JacobiSolver
    Function forcing_func = [&forcing_parser](double x, double y) -> double {
        return (*forcing_parser)(x, y);
    };
    
    Function exact_func = [&exact_parser](double x, double y) -> double {
        return (*exact_parser)(x, y);
    };

    // Create grid and solver
    MPIGrid grid(n, rank, size);
    JacobiSolver solver(grid, forcing_func, tolerance, max_iter);

    // Solve
    double start_time = MPI_Wtime();
    int iterations = solver.solve();
    double end_time = MPI_Wtime();

    // Compute L2 error (only if exact solution provided)
    double l2_error = 0.0;
    if (exact_parser->defined()) {
        l2_error = solver.compute_l2_error(exact_func);
    }

    if (rank == 0) {
        std::cout << "\n=== Results ===" << std::endl;
        std::cout << "Grid size: " << n << " x " << n << std::endl;
        std::cout << "MPI processes: " << size << std::endl;
        std::cout << "Forcing term: " << forcing_expr << std::endl;
        std::cout << "Iterations: " << iterations << std::endl;
        std::cout << "Time: " << end_time - start_time << " seconds" << std::endl;
        if (exact_parser->defined()) {
            std::cout << "L2 error: " << l2_error << std::endl;
        }
    }

    // Export to VTK
    std::vector<double> global_solution;
    grid.gather_solution(global_solution);

    if (rank == 0) {
        std::string filename = "solution_n" + std::to_string(n) + ".vtk";
        VTKWriter::write(filename, grid.n, grid.h, global_solution);
        std::cout << "Solution written to " << filename << std::endl;
    }

    MPI_Finalize();
    return 0;
}