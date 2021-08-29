#pragma once

#include "optimizationtools/info.hpp"
#include "optimizationtools/utils.hpp"
#include "optimizationtools/indexed_map.hpp"
#include "optimizationtools/doubly_indexed_map.hpp"

#include <random>
#include <set>

namespace stablesolver
{

typedef int64_t VertexId; // v
typedef int64_t VertexPos; // v_pos
typedef int64_t EdgeId; // e
typedef int64_t EdgePos; // e_pos
typedef int64_t Weight; // w
typedef int64_t ComponentId; // c
typedef int64_t Counter;
typedef int64_t Seed;

class Solution;

/******************************************************************************/

struct VertexEdge
{
    EdgeId e;
    VertexId v;
};

struct Vertex
{
    VertexId id;
    Weight weight = 1;
    ComponentId component = -1;
    std::vector<VertexEdge> edges;
};

struct Edge
{
    EdgeId id;
    VertexId v1;
    VertexId v2;
    ComponentId component = -1;
};

struct Component
{
    ComponentId id;
    std::vector<EdgeId> edges;
    std::vector<VertexId> vertices;
};

class Instance 
{

public:

    /** Create an instance from a file. */
    Instance(std::string instance_path, std::string format);

    /** Create an instance manually. */
    Instance(VertexId number_of_vertices);
    /** Set the weight of vertex 'v' to 'weight'. */
    void set_weight(VertexId v, Weight weight);
    /** Add an edge between vertex 'v1' and vertex 'v2'. */
    void add_edge(VertexId v1, VertexId v2);
    /** Set the weight of all vertices to 1. */
    void set_unweighted();
    /** Compute the connected components of the instance. */
    void compute_components();

    /** Create the complementary instance. */
    Instance complementary();

    /*
     * Getters.
     */

    /** Get the number of vertices. */
    inline VertexId number_of_vertices() const { return vertices_.size(); }
    /** Get the number of edges. */
    inline EdgeId number_of_edges() const { return edges_.size(); }
    /** Get the number of connected components. */
    inline ComponentId number_of_components() const { return components_.size(); }

    /** Get vertex 'v'. */
    inline const Vertex& vertex(VertexId v) const { return vertices_[v]; }
    /** Get edge 'e'. */
    inline const Edge& edge(EdgeId e) const { return edges_[e]; }
    /** Get connected component 'c'. */
    inline const Component& component(ComponentId c) const { return components_[c]; }

    /** Get the degree of vertex 'v'. */
    inline VertexId degree(VertexId v) const { return vertices_[v].edges.size(); }
    /** Get the maximum vertex degree of the instance. */
    inline VertexId maximum_degree() const { return maximum_degree_; }
    /** Get the total weight of the instance. */
    inline Weight total_weight() const { return total_weight_; }

    /** Write the instance to a file. */
    void write(std::string instance_path, std::string format);

private:

    /*
     * Attributes.
     */

    /** Name of the instance. */
    std::string name_ = "";
    /** Vertices. */
    std::vector<Vertex> vertices_;
    /** Edges. */
    std::vector<Edge> edges_;
    /** Connected components. */
    std::vector<Component> components_;
    /** Maximum vertex degree of the instance. */
    VertexId maximum_degree_ = 0;
    /** Total weight of the instance. */
    Weight total_weight_ = 0;

    /*
     * Private methods.
     */

    /** Read an instance file in 'dimacs1992' format. */
    void read_dimacs1992(std::ifstream& file);
    /** Read an instance file in 'dimacs2010' format. */
    void read_dimacs2010(std::ifstream& file);
    /** Read an instance file in 'matrixmarket' format. */
    void read_matrixmarket(std::ifstream& file);
    /** Read an instance file in 'chaco' format. */
    void read_chaco(std::ifstream& file);

};

std::ostream& operator<<(std::ostream &os, const Instance& ins);

}

