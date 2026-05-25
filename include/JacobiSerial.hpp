#ifndef JACOBISERIAL_HPP
#define JACOBISERIAL_HPP

#include <vector>
#include <functional>

using Function = std::function<double(double, double)>;

class JacobiSerial {
    private:
        Function forcing_term;

        std::vector<double> U;
        std::vector<double> U_new;

        void initialize_boundary_conditions();

    public:
        double tolerance;
        int max_iterations;
        int n;
        double h;

        // Constructor
        JacobiSerial(int n,
                    Function f,
                    double tol,
                    int max_iter);

        void iterate();
        double compute_error() const;
        int solve();
        double compute_l2_error(Function exact_solution) const;
        const std::vector<double>& get_solution() const { return U; }

    };

#endif