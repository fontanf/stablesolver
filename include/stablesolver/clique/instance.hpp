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

    /** Create an instance from an AbstrctGraph. */
    Instance(const std::shared_ptr<const optimizationtools::AbstractGraph>& abstract_graph);

    /** Create an instance from a file. */
    Instance(
            const std::string& instance_path,
            const std::string& format);

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
    std::shared_ptr<const optimizationtools::AbstractGraph> graph_ = nullptr;

    /**
     * Adjacency list graph.
     *
     * 'nullptr' if 'graph_' is not an AdjacencyList graph.
     */
    const optimizationtools::AdjacencyListGraph* adjacency_list_graph_ = nullptr;

};

}
}
