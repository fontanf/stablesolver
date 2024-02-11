#pragma once

#include "optimizationtools/containers/indexed_set.hpp"
#include "optimizationtools/graph/abstract_graph.hpp"
#include "optimizationtools/graph/adjacency_list_graph.hpp"

#include <memory>

namespace stablesolver
{
namespace clique
{

using VertexId = optimizationtools::VertexId;
using VertexPos = optimizationtools::VertexPos;
using EdgeId = optimizationtools::EdgeId;
using Weight = optimizationtools::Weight;
using Counter = int64_t;
using Seed = int64_t;

class Instance
{

public:

    /*
     * Constructors and destructor
     */

    /** Create an instance from a file. */
    Instance(
            const std::string& instance_path,
            const std::string& format);

    /** Create an instance manually. */
    Instance(VertexId number_of_vertices);

    /** Add a vertex. */
    VertexId add_vertex(Weight weight = 1) { return adjacency_list_graph_->add_vertex(weight); }

    /** Set the weight of vertex 'v' to 'weight'. */
    void set_weight(
            VertexId vertex_id,
            Weight weight)
    {
        return adjacency_list_graph_->set_weight(vertex_id, weight);
    }

    /** Add an edge between vertex 'vertex_id_1' and vertex 'vertex_id_2'. */
    void add_edge(
            VertexId vertex_id_1,
            VertexId vertex_id_2)
    {
        adjacency_list_graph_->add_edge(vertex_id_1, vertex_id_2);
    }

    /** Set the weight of all vertices to 1. */
    void set_unweighted() { adjacency_list_graph_->set_unweighted(); }

    /** Create an instance from an AbstrctGraph. */
    Instance(const optimizationtools::AbstractGraph& abstract_graph);

    /** Create the complementary instance. */
    Instance complementary();

    /*
     * Getters
     */

    /** Get graph. */
    inline const optimizationtools::AbstractGraph* graph() const { return graph_.get(); }

    /** Get the adjacency list graph. */
    inline const optimizationtools::AdjacencyListGraph* adjacency_list_graph() const { return adjacency_list_graph_; }

    Weight update_core(
            optimizationtools::IndexedSet& relevant_vertices,
            Weight weight) const;

    /*
     * Export
     */

    /** Print the instance. */
    std::ostream& format(
            std::ostream& os,
            int verbosity_level = 1) const;

private:

    /*
     * Private attributes
     */

    /** Graph. */
    std::unique_ptr<optimizationtools::AbstractGraph> graph_ = nullptr;

    /**
     * Adjacency list graph.
     *
     * 'nullptr' if 'graph_' is not an AdjacencyList graph.
     */
    optimizationtools::AdjacencyListGraph* adjacency_list_graph_ = nullptr;

};

}
}
