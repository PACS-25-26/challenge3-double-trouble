#ifndef VTKWRITER_HPP
#define VTKWRITER_HPP

#include <string>
#include <vector>

class VTKWriter {
public:
    static void write(const std::string& filename, 
                     int n, 
                     double h,
                     const std::vector<double>& solution);
};

#endif
