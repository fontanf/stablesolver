#include "stablesolver/clique/instance.hpp"

#include <iomanip>
#include <ostream>

using namespace stablesolver::clique;

Instance::Instance(
        const std::string& instance_path,
        const std::string& format)
{
    optimizationtools::AdjacencyListGraphBuilder graph_builder;
    graph_builder.read(instance_path, format);
    graph_ = std::shared_ptr<const optimizationtools::AbstractGraph>(
                new optimizationtools::AdjacencyListGraph(graph_builder.build()));
    adjacency_list_graph_ = static_cast<const optimizationtools::AdjacencyListGraph*>(graph_.get());
}

Instance::Instance(const std::shared_ptr<const optimizationtools::AbstractGraph>& abstract_graph):
    graph_(abstract_graph),
    adjacency_list_graph_(dynamic_cast<const optimizationtools::AdjacencyListGraph*>(graph_.get())) { }

Weight Instance::update_core(
        optimizationtools::IndexedSet& relevant_vertices,
        Weight weight) const
{
    std::vector<Weight> best_values(graph()->number_of_vertices(), 0);
    std::vector<VertexId> vertex_queue;
    for (VertexId vertex_id: relevant_vertices) {
        best_values[vertex_id] = graph()->weight(vertex_id);
        for (auto it = graph()->neighbors_begin(vertex_id);
                it != graph()->neighbors_end(vertex_id);
                ++it) {
            if (relevant_vertices.contains(*it))
                best_values[vertex_id] += graph()->weight(*it);
        }
        if (best_values[vertex_id] <= weight)
            vertex_queue.push_back(vertex_id);
    }
    while (!vertex_queue.empty()) {
        VertexId vertex_id = vertex_queue.back();
        relevant_vertices.remove(vertex_id);
        vertex_queue.pop_back();
        for (auto it = graph()->neighbors_begin(vertex_id);
                it != graph()->neighbors_end(vertex_id);
                ++it) {
            if (best_values[*it] <= weight)
                continue;
            best_values[*it] -= graph()->weight(vertex_id);
            if (best_values[*it] <= weight)
                vertex_queue.push_back(*it);
        }
    }
    Weight bound = 0;
    for (Weight value: best_values)
        if (bound < value)
            bound = value;
    return bound;
}

std::ostream& Instance::format(
        std::ostream& os,
        int verbosity_level) const
{
    if (verbosity_level >= 1) {
        double density = (double)graph()->number_of_edges() * 2
            / graph()->number_of_vertices()
            / graph()->number_of_vertices();
        os
            << "Number of vertices:              " << graph()->number_of_vertices() << std::endl
            << "Number of edges:                 " << graph()->number_of_edges() << std::endl
            << "Density:                         " << density << std::endl
            << "Average degree:                  " << (double)graph()->number_of_edges() * 2 / graph()->number_of_vertices() << std::endl
            << "Highest degree:                  " << graph()->highest_degree() << std::endl
            << "Total weight:                    " << graph()->total_weight() << std::endl
            ;
    }

    if (verbosity_level >= 2) {
        os << std::endl
            << std::setw(12) << "VertexId"
            << std::setw(12) << "Weight"
            << std::setw(12) << "Degree"
            << std::endl
            << std::setw(12) << "--------"
            << std::setw(12) << "------"
            << std::setw(12) << "------"
            << std::endl;
        for (VertexId vertex_id = 0;
                vertex_id < graph()->number_of_vertices();
                ++vertex_id) {
            os
                << std::setw(12) << vertex_id
                << std::setw(12) << graph()->weight(vertex_id)
                << std::setw(12) << graph()->degree(vertex_id)
                << std::endl;
        }
    }

    return os;
}
