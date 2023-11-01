#pragma once

#include "optimizationtools/utils/info.hpp"

namespace stablesolver
{
namespace stable
{

using VertexId = int32_t;
using VertexPos = int32_t;
using EdgeId = int64_t;
using EdgePos = int64_t;
using Weight = int64_t;
using ComponentId = int64_t;
using Penalty = int16_t;
using Counter = int64_t;
using Seed = int64_t;

class Solution;

/**
 * Structure that stores the information of a neighbor for a considered vertex.
 */
struct VertexEdge
{
    /** Id of the edge. */
    EdgeId edge_id;

    /** Id of the neighbor. */
    VertexId vertex_id;
};

/**
 * Structure that stores the information for a vertex.
 */
struct Vertex
{
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
    /** Id of the first end of the edge. */
    VertexId vertex_id_1;

    /** Id of the second end of the edge. */
    VertexId vertex_id_2;

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

struct UnreductionInfo
{
    /** Pointer to the original instance. */
    const Instance* original_instance = nullptr;

    /** For each vertex, unreduction operations. */
    std::vector<UnreductionOperations> unreduction_operations;

    /** Mandatory vertices (from the original instance). */
    std::vector<VertexId> mandatory_vertices;

    /**
     * Weight to add to a solution of the reduced instance to get the weight of
     * the corresponding solution of the original instance.
     */
    Weight extra_weight;
};

/**
 * Structure passed as parameters of the reduction algorithm and the other
 * algorithm to determine whether and how to reduce.
 */
struct ReductionParameters
{
    /** Boolean indicating if the reduction should be performed. */
    bool reduce = true;

    /** Maximum number of rounds. */
    Counter maximum_number_of_rounds = 10;
};

/**
 * Instance class for a Maximum-Weight Independent Set problem.
 */
class Instance
{

public:

    /** Create the complementary instance. */
    Instance complementary();

    /** Reduce. */
    Instance reduce(ReductionParameters parameters) const;

    /*
     * Getters.
     */

    /** Get the number of vertices. */
    inline VertexId number_of_vertices() const { return vertices_.size(); }

    /** Get the number of edges. */
    inline EdgeId number_of_edges() const { return edges_.size(); }

    /** Get the number of connected components. */
    inline ComponentId number_of_components() const { return components_.size(); }

    /** Get a vertex. */
    inline const Vertex& vertex(VertexId vertex_id) const { return vertices_[vertex_id]; }

    /** Get an edge. */
    inline const Edge& edge(EdgeId edge_id) const { return edges_[edge_id]; }

    /** Get connected a component. */
    inline const Component& component(ComponentId c) const { return components_[c]; }

    /** Get the degree of a vertex. */
    inline VertexId degree(VertexId vertex_id) const { return vertices_[vertex_id].edges.size(); }

    /** Get the maximum vertex degree of the instance. */
    inline VertexId maximum_degree() const { return maximum_degree_; }

    /** Get the total weight. */
    inline Weight total_weight() const { return total_weight_; }

    /*
     * Reduction information
     */

    /** Get the original instance. */
    inline const Instance* original_instance() const { return (is_reduced())? unreduction_info().original_instance: this; }

    /** Return 'true' iff the instance is a reduced instance. */
    inline bool is_reduced() const { return unreduction_info_.original_instance != nullptr; }

    /** Get the unreduction info of the instance; */
    inline const UnreductionInfo& unreduction_info() const { return unreduction_info_; }

    /*
     * Export
     */

    /** Print the instance. */
    std::ostream& print(
            std::ostream& os,
            int verbose = 1) const;

    /** Write the instance to a file. */
    void write(
            std::string instance_path,
            std::string format);

    /*
     * Checkers
     */

    /** Check if vertex index 'v' is within the correct range. */
    inline void check_vertex_index(VertexId vertex_id) const
    {
        if (vertex_id < 0 || vertex_id >= number_of_vertices())
            throw std::out_of_range(
                    "Invalid vertex index: \"" + std::to_string(vertex_id) + "\"."
                    + " Vertex indices should belong to [0, "
                    + std::to_string(number_of_vertices() - 1) + "].");
    }

private:

    /*
     * Attributes
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

    /** Total weight. */
    Weight total_weight_ = 0;

    /** Reduction structure. */
    UnreductionInfo unreduction_info_;

    /*
     * Private methods
     */

    /*
     * Reductions
     */

    /**
     * Perform pendant vertices reduction.
     *
     * See:
     * - "Accelerating Local Search for the Maximum Independent Set Problem"
     *   (Dahlum et al., 2016)
     *   https://doi.org/10.1007/978-3-319-38851-9_9
     */
    bool reduce_pendant_vertices();

    /**
     * Perform isolated vertex removal reduction.
     *
     * See:
     * - "Exactly Solving the Maximum Weight Independent Set Problem on Large
     *   Real-World Graphs" (Lamm et al., 2019)
     *   https://doi.org/10.1137/1.9781611975499.12
     */
    bool reduce_isolated_vertex_removal();

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
    bool reduce_vertex_folding();

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
    bool reduce_twin();

    /**
     * Perform domination reduction.
     *
     * See:
     * - "Branch-and-reduce exponential/FPT algorithms in practice: A case
     *   study of vertex cover" (Akibaa et Iwata, 2016)
     *   https://doi.org/10.1016/j.tcs.2015.09.023
     * - "Exactly Solving the Maximum Weight Independent Set Problem on Large
     *   Real-World Graphs" (Lamm et al., 2019)
     *   https://doi.org/10.1137/1.9781611975499.12
     */
    bool reduce_domination();

    /**
     * Perform unconfined reduction.
     *
     * The unconfined reduction rule is a generalization of the dominance and
     * the satellite reduction rules.
     *
     * See:
     * - "Confining sets and avoiding bottleneck cases: A simple maximum
     *   independent set algorithm in degree-3 graphs" (Xiao et
     *   HiroshiNagamochi, 2013)
     *   https://doi.org/10.1016/j.tcs.2012.09.022
     * - "Accelerating Local Search for the Maximum Independent Set Problem"
     *   (Dahlum et al., 2016)
     *   https://doi.org/10.1007/978-3-319-38851-9_9
     * - "Exactly Solving the Maximum Weight Independent Set Problem on Large
     *   Real-World Graphs" (Lamm et al., 2019)
     *   https://doi.org/10.1137/1.9781611975499.12
     */
    bool reduce_unconfined();

    friend class InstanceBuilder;

};

void init_display(
        const Instance& instance,
        optimizationtools::Info& info);

}
}

