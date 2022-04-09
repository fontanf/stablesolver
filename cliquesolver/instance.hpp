#pragma once

#include "optimizationtools/utils/info.hpp"
#include "optimizationtools/containers/indexed_set.hpp"
#include "optimizationtools/graph/abstract_graph.hpp"
#include "optimizationtools/graph/adjacency_list_graph.hpp"

#include <memory>

namespace cliquesolver
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
     * Constructors and destructor.
     */

    /** Create an instance from a file. */
    Instance(std::string instance_path, std::string format);

    /** Create an instance manually. */
    Instance(VertexId number_of_vertices);
    /** Add a vertex. */
    VertexId add_vertex(Weight weight = 1) { return adjacency_list_graph_->add_vertex(weight); }
    /** Set the weight of vertex 'v' to 'weight'. */
    void set_weight(VertexId v, Weight weight) { return adjacency_list_graph_->set_weight(v, weight); }
    /** Add an edge between vertex 'v1' and vertex 'v2'. */
    void add_edge(VertexId v1, VertexId v2) { adjacency_list_graph_->add_edge(v1, v2); }
    /** Set the weight of all vertices to 1. */
    void set_unweighted() { adjacency_list_graph_->set_unweighted(); }

    /** Create an instance from an AbstrctGraph. */
    Instance(const optimizationtools::AbstractGraph& abstract_graph);

    /** Create the complementary instance. */
    Instance complementary();

    /*
     * Getters.
     */

    inline const optimizationtools::AbstractGraph* graph() const { return graph_.get(); }

    Weight update_core(
            optimizationtools::IndexedSet& relevant_vertices,
            Weight weight) const;

    /*
     * Export.
     */

    /** Write the instance to a file. */
    void write(std::string instance_path, std::string format);

private:

    /*
     * Private attributes.
     */

    std::unique_ptr<optimizationtools::AbstractGraph> graph_ = nullptr;

    optimizationtools::AdjacencyListGraph* adjacency_list_graph_ = nullptr;

};

std::ostream& operator<<(std::ostream &os, const Instance& ins);

void init_display(
        const Instance& instance,
        optimizationtools::Info& info);

}

