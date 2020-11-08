#pragma once

#include "optimizationtools/info.hpp"
#include "optimizationtools/utils.hpp"
#include "optimizationtools/indexed_map.hpp"
#include "optimizationtools/doubly_indexed_map.hpp"

#include <random>
#include <set>

#define TOL 0.0000001

namespace stablesolver
{

using optimizationtools::Info;

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

    /** Constructor from file. */
    Instance(std::string filepath, std::string format);

    /** Manual constructor. */
    Instance(VertexId vertex_number);
    void set_weight(VertexId v, Weight w);
    void add_edge(VertexId v1, VertexId v2);
    void set_unweighted();
    void compute_components();

    Instance complementary();

    /** Getters. */

    inline VertexId        vertex_number() const { return vertices_.size(); }
    inline EdgeId            edge_number() const { return edges_.size(); }
    inline ComponentId  component_number() const { return components_.size(); }

    inline const Vertex& vertex(VertexId v) const { return vertices_[v]; }
    inline const Edge& edge(EdgeId e) const { return edges_[e]; }
    inline const Component& component(ComponentId c) const { return components_[c]; }

    inline VertexId degree(VertexId v) const { return vertices_[v].edges.size(); }
    inline VertexId degree_max() const { return degree_max_; }
    inline Weight total_weight() const { return weight_total_; }

    /** Export. */
    void write(std::string filepath, std::string format);

private:

    /**
     * Attributes.
     */

    std::string name_ = "";
    std::vector<Vertex> vertices_;
    std::vector<Edge> edges_;
    std::vector<Component> components_;
    VertexId degree_max_ = 0;
    Weight weight_total_;

    /**
     * Private methods.
     */

    void read_dimacs1992(std::ifstream& file);
    void read_dimacs2010(std::ifstream& file);
    void read_matrixmarket(std::ifstream& file);

};

std::ostream& operator<<(std::ostream &os, const Instance& ins);

}

