#include "cliquesolver/instance.hpp"

void cliquesolver::init_display(
        const Instance& instance,
        optimizationtools::Info& info)
{
    VertexId n = instance.number_of_vertices();
    EdgeId m = instance.number_of_edges();
    VER(info,
               "=====================================" << std::endl
            << "            Clique Solver            " << std::endl
            << "=====================================" << std::endl
            << std::endl
            << "Instance" << std::endl
            << "--------" << std::endl
            << "Number of vertices:              " << n << std::endl
            << "Number of edges:                 " << m << std::endl
            << "Density:                         " << (double)m * 2 / n / (n - 1) << std::endl
            << "Average degree:                  " << (double)instance.number_of_edges() * 2 / n << std::endl
            << "Maximum degree:                  " << instance.maximum_degree() << std::endl
            << "Number of connected components:  " << instance.number_of_components() << std::endl
            << std::endl);
}
