#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <stdexcept>

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
 * Instance class for a maximum-Weight independent set problem.
 */
class Instance
{

public:

    /** Create the complementary instance. */
    const Instance complementary();

    /*
     * Getters
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
    inline VertexId highest_degree() const { return highest_degree_; }

    /** Get the total weight. */
    inline Weight total_weight() const { return total_weight_; }

    /*
     * Export
     */

    /** Print the instance. */
    std::ostream& format(
            std::ostream& os,
            int verbosity_level = 1) const;

    /** Write the instance to a file. */
    void write(
            const std::string& instance_path,
            const std::string& format);

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
    VertexId highest_degree_ = 0;

    /** Total weight. */
    Weight total_weight_ = 0;

    /*
     * Private methods
     */

    friend class InstanceBuilder;

};

}
}
