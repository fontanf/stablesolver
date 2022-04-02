#pragma once

#include "optimizationtools/info.hpp"
#include "optimizationtools/utils.hpp"

#include <random>
#include <set>

namespace stablesolver
{

typedef int32_t VertexId; // v
typedef int32_t VertexPos; // v_pos
typedef int64_t EdgeId; // e
typedef int64_t EdgePos; // e_pos
typedef int64_t Weight; // w
typedef int64_t ComponentId; // c
typedef int16_t Penalty;
typedef int64_t Counter;
typedef int64_t Seed;

class Solution;

/**
 * Structure that stores the information of a neighbor for a considered vertex.
 */
struct VertexEdge
{
    /** Id of the edge. */
    EdgeId e;
    /** Id of the neighbor. */
    VertexId v;
};

/**
 * Structure that stores the information for a vertex.
 */
struct Vertex
{
    /** Unique id of the vertex. */
    VertexId id;
    /** Weight of the vertex. */
    Weight weight = 1;
    /** Id of the connected component of the vertex. */
    ComponentId component = -1;
    /** Neighbors of the vertex. */
    std::vector<VertexEdge> edges;
};

/**
 * Structure that stores the information for an edge.
 */
struct Edge
{
    /** Unique id of the edge. */
    EdgeId id;
    /** Id of the first end of the edge. */
    VertexId v1;
    /** Id of the second end of the edge. */
    VertexId v2;
    /** Id of the connected component of the edge. */
    ComponentId component = -1;
};

/**
 * Structure that stores the information for a connected component.
 */
struct Component
{
    /** Unique id of the connected component. */
    ComponentId id;
    /** Ids of the edges in the connected component. */
    std::vector<EdgeId> edges;
    /** Ids of the vertices in the connected component. */
    std::vector<VertexId> vertices;
};

/**
 * Structure that stores the unreduction operation for a considered vertex.
 */
struct UnreductionOperations
{
    /**
     * List of vertices from the original graph to add if the considered vertex
     * is in the solution of the reduced instance.
     */
    std::vector<VertexId> in;

    /**
     * List of vertices from the original graph to add if the considered vertex
     * is NOT in the solution of the reduced instance.
     */
    std::vector<VertexId> out;
};

class Instance;

/**
 * Structure returned by the reduction operations.
 */
struct ReductionOutput
{
    /** Pointer to the reduced instance. */
    Instance* instance = nullptr;
    /** For each vertex, unreduction operations. */
    std::vector<UnreductionOperations> unreduction_operations;
    /** Mandatory vertices (from the original instance). */
    std::vector<VertexId> mandatory_vertices;
};

class Instance
{

public:

    /*
     * Constructors and destructor.
     */

    /** Create an instance from a file. */
    Instance(std::string instance_path, std::string format);

    /** Create an instance manually. */
    Instance(VertexId number_of_vertices);
    /** Set the weight of vertex 'v' to 'weight'. */
    void set_weight(VertexId v, Weight weight);
    /** Add an edge between vertex 'v1' and vertex 'v2'. */
    void add_edge(VertexId v1, VertexId v2, int check_duplicate = 0);
    /** Set the weight of all vertices to 1. */
    void set_unweighted();
    /** Compute the connected components of the instance. */
    void compute_components();

    /** Create the complementary instance. */
    Instance complementary();

    void reduce();

    /** Destructor. */
    ~Instance() { if (reduction_output_.instance != this) delete reduction_output_.instance; }

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

    /*
     * Reduction information.
     */

    /**
     * Get a pointer to the reduced instance.
     *
     * Return 'nullptr' if no reduction has been applied.
     */
    inline Instance* reduced_instance() const { return reduction_output_.instance; }
    /**
     * Get the weight to add to a solution of the reduced instance to get the
     * weight of the corresponding solution of the original instance.
     **/
    inline Weight extra_weight() const { return extra_weight_; }
    /** Get the unreduction operations of vertex 'v'. */
    inline const UnreductionOperations unreduction_operations(VertexId v) const { return reduction_output_.unreduction_operations[v]; }
    /** Get the list of mandatory vertices from the orignal instance. */
    inline const std::vector<VertexId>& mandatory_vertices() const { return reduction_output_.mandatory_vertices; }

    /*
     * Export.
     */

    /** Write the instance to a file. */
    void write(std::string instance_path, std::string format);

    /*
     * Checkers.
     */

    /** Check if vertex index 'v' is within the correct range. */
    inline void check_vertex_index(VertexId v) const
    {
        if (v < 0 || v >= number_of_vertices())
            throw std::out_of_range(
                    "Invalid vertex index: \"" + std::to_string(v) + "\"."
                    + " Vertex indices should belong to [0, "
                    + std::to_string(number_of_vertices() - 1) + "].");
    }

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
     * Reduction structures.
     */

    /** Reduction output. */
    ReductionOutput reduction_output_;
    /**
     * Weight to add to a solution of the reduced instance to get the weight of
     * the corresponding solution of the original instance.
     **/
    Weight extra_weight_;

    /*
     * Private methods.
     */

    /*
     * Read input file.
     */

    /** Read an instance file in 'dimacs1992' format. */
    void read_dimacs1992(std::ifstream& file);
    /** Read an instance file in 'dimacs2010' format. */
    void read_dimacs2010(std::ifstream& file);
    /** Read an instance file in 'matrixmarket' format. */
    void read_matrixmarket(std::ifstream& file);
    /** Read an instance file in 'chaco' format. */
    void read_chaco(std::ifstream& file);

    /*
     * Reductions.
     */

    /**
     * Perform pendant vertices reduction.
     *
     * See:
     * - "Accelerating Local Search for the Maximum Independent Set Problem"
     *   (Dahlum et al., 2016)
     *   https://doi.org/10.1007/978-3-319-38851-9_9
     */
    static ReductionOutput reduce_pendant_vertices(
            const ReductionOutput& reduction_output_old);

    /**
     * Perform isolated vertex removal reduction.
     *
     * See:
     * - "Exactly Solving the Maximum Weight Independent Set Problem on Large
     *   Real-World Graphs" (Lamm et al., 2019)
     *   https://doi.org/10.1137/1.9781611975499.12
     */
    static ReductionOutput reduce_isolated_vertex_removal(
            const ReductionOutput& reduction_output_old);
    /**
     * Perform vertex folding reduction.
     *
     * See:
     * - "Branch-and-reduce exponential/FPT algorithms in practice: A case
     *   study of vertex cover" (Akibaa et Iwata, 2016)
     *   https://doi.org/10.1016/j.tcs.2015.09.023
     * - "Exactly Solving the Maximum Weight Independent Set Problem on Large
     *   Real-World Graphs" (Lamm et al., 2019)
     *   https://doi.org/10.1137/1.9781611975499.12
     */
    static ReductionOutput reduce_vertex_folding(
            const ReductionOutput& reduction_output_old);
    /**
     * Perform twin reduction.
     *
     * See:
     * - "Branch-and-reduce exponential/FPT algorithms in practice: A case
     *   study of vertex cover" (Akibaa et Iwata, 2016)
     *   https://doi.org/10.1016/j.tcs.2015.09.023
     * - "Exactly Solving the Maximum Weight Independent Set Problem on Large
     *   Real-World Graphs" (Lamm et al., 2019)
     *   https://doi.org/10.1137/1.9781611975499.12
     */
    static ReductionOutput reduce_twin(
            const ReductionOutput& reduction_output_old);
    /**
     * Perform domination reduction.
     *
     * See:
     * - "Branch-and-reduce exponential/FPT algorithms in practice: A case
     *   study of vertex cover" (Akibaa et Iwata, 2016)
     *   https://doi.org/10.1016/j.tcs.2015.09.023
     */
    static ReductionOutput reduce_domination(
            const ReductionOutput& reduction_output_old);

};

std::ostream& operator<<(std::ostream &os, const Instance& ins);

void init_display(
        const Instance& instance,
        optimizationtools::Info& info);

}

