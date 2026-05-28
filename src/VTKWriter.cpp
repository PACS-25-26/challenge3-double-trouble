#include "VTKWriter.hpp"
#include <fstream>
#include <iomanip>

/**
 * @brief Write solution to VTK file
 * @param filename Output filename
 * @param n Grid size (n x n)
 * @param h Mesh spacing
 * @param solution Global solution vector (n*n values)
 */
void VTKWriter::write(const std::string& filename, 
                      int n, 
                      double h,
                      const std::vector<double>& solution) {
    std::ofstream file(filename);
    
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }
    
    file << "# vtk DataFile Version 3.0\n";
    file << "Jacobi Solver Solution\n";
    file << "ASCII\n";
    file << "DATASET STRUCTURED_POINTS\n";
    file << "DIMENSIONS " << n << " " << n << " 1\n";
    file << "ORIGIN 0 0 0\n";
    file << "SPACING " << h << " " << h << " 1\n"; 
    file << "POINT_DATA " << n * n << "\n";
    file << "SCALARS solution double 1\n";
    file << "LOOKUP_TABLE default\n";
    
    // Write solution values
    file << std::scientific << std::setprecision(8);
    for (int i = 0; i < n * n; ++i) {
        file << solution[i] << "\n";
    }
    
    file.close();
}