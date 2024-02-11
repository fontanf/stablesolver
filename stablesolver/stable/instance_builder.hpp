#pragma once

#include "stablesolver/stable/instance.hpp"

namespace stablesolver
{
namespace stable
{

class InstanceBuilder
{

public:

    /** Constructor. */
    InstanceBuilder() { }

    /** Add vertices. */
    void add_vertices(VertexId number_of_vertices);

    /** Add a vertex. */
    void add_vertex(Weight weight = 1);

    /** Set the weight of vertex 'v' to 'weight'. */
    void set_weight(
            VertexId vertex_id,
            Weight weight);

    /** Add an edge between two vertices. */
    void add_edge(
            VertexId vertex_id_1,
            VertexId vertex_id_2,
            int check_duplicate = 0);

    /** Set the weight of all vertices to 1. */
    void set_unweighted();

    /** Read an instance from a file. */
    void read(
            std::string instance_path,
            std::string format);

    /*
     * Build
     */

    /** Build. */
    Instance build();

private:

    /*
     * Private methods
     */

    /** Compute the maximum degree. */
    void compute_highest_degree();

    /** Compute the total weight. */
    void compute_total_weight();

    /** Compute the connected components of the instance. */
    void compute_components();

    /*
     * Read input file
     */

    /** Read an instance file in 'dimacs1992' format. */
    void read_dimacs1992(std::ifstream& file);

    /** Read an instance file in 'dimacs2010' format. */
    void read_dimacs2010(std::ifstream& file);

    /** Read an instance file in 'matrixmarket' format. */
    void read_matrixmarket(std::ifstream& file);

    /** Read an instance file in 'chaco' format. */
    void read_chaco(std::ifstream& file);

    /** Read an instance file in 'snap' format. */
    void read_snap(std::ifstream& file);

    /*
     * Private attributes
     */

    /** Instance. */
    Instance instance_;

};

}
}
