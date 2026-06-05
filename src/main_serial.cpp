#include "JacobiSerial.hpp"
#include "VTKWriter.hpp"
#include <iostream>
#include <cmath>
#include <muParser.h>
#include <memory>
#include <chrono>

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
            if (!is_defined) return 0.0;
            
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
    // Fixed parameters
    const double tolerance = 1e-7;
    const int max_iter = 1e6;

    // Parse command line arguments
    if (argc < 3) {
        std::cerr << "Wrong number of arguments! Right usage is: " << argv[0]
                  << " <n> \"<forcing_term>\" [\"<exact_solution>\"]" << std::endl;
        return 1;
    }

    int n = std::atoi(argv[1]);
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
        std::cerr << "Parser error: " << e.GetMsg() << std::endl;
        return 1;
    }

    // Create lambda wrappers
    Function forcing_func = [&forcing_parser](double x, double y) -> double {
        return (*forcing_parser)(x, y);
    };
    
    Function exact_func = [&exact_parser](double x, double y) -> double {
        return (*exact_parser)(x, y);
    };

    // Create solver
    JacobiSerial solver(forcing_func, tolerance, max_iter, n);

    // Solve
    auto start_time = std::chrono::high_resolution_clock::now();
    int iterations = solver.solve();
    auto end_time = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration<double>(end_time - start_time).count();

    // Compute L2 error (only if exact solution provided)
    double l2_error = 0.0;
    if (exact_parser->defined()) {
        l2_error = solver.compute_l2_error(exact_func);
    }

    std::cout << "\n=== Results ===" << std::endl;
    std::cout << "Grid size: " << n << " x " << n << std::endl;
    std::cout << "Forcing term: " << forcing_expr << std::endl;
    std::cout << "Iterations: " << iterations << std::endl;
    std::cout << "Time: " << elapsed << " seconds" << std::endl;
    if (exact_parser->defined()) {
        std::cout << "L2 error: " << l2_error << std::endl;
    }

    // Export to VTK
    std::string filename = "solution_serial_n" + std::to_string(n) + ".vtk";
    VTKWriter::write(filename, solver.n, solver.h, solver.get_solution());
    std::cout << "Solution written to " << filename << std::endl;
   
    return 0;
}