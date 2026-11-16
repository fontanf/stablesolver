#pragma once

#include "optimizationtools/containers/indexed_set.hpp"
#include "optimizationtools/graph/graph.hpp"

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

    /** Create an instance from a graph. */
    Instance(const optimizationtools::Graph& graph);

    /**
     * Create an instance from a file.
     *
     * The graph is a CliqueGraph for the 'cliquegraph' format, an
     * AdjacencyListGraph otherwise.
     */
    Instance(
            const std::string& instance_path,
            const std::string& format);

    /*
     * Getters
     */

    /** Get graph. */
    inline const optimizationtools::Graph* graph() const { return &graph_; }

    /**
     * Get the adjacency list graph.
     *
     * 'nullptr' if the graph is not an AdjacencyListGraph.
     */
    inline const optimizationtools::AdjacencyListGraph* adjacency_list_graph() const { return graph_.adjacency_list_graph(); }

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
    optimizationtools::Graph graph_;

};

}
}
