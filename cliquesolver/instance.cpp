#include "cliquesolver/instance.hpp"

using namespace cliquesolver;

Instance::Instance(
        std::string instance_path,
        std::string format):
    graph_(std::unique_ptr<optimizationtools::AbstractGraph>(
                new optimizationtools::AdjacencyListGraph(instance_path, format))),
    adjacency_list_graph_(static_cast<optimizationtools::AdjacencyListGraph*>(graph_.get())) { }

Instance::Instance(
        VertexId number_of_vertices):
    graph_(std::unique_ptr<optimizationtools::AbstractGraph>(
                new optimizationtools::AdjacencyListGraph(number_of_vertices))),
    adjacency_list_graph_(static_cast<optimizationtools::AdjacencyListGraph*>(graph_.get())) { }

Instance::Instance(const optimizationtools::AbstractGraph& abstract_graph):
    graph_(std::unique_ptr<optimizationtools::AbstractGraph>(abstract_graph.clone())) { }

Instance Instance::complementary()
{
    Instance instance(graph()->number_of_vertices());
    optimizationtools::IndexedSet neighbors(graph()->number_of_vertices());

    for (VertexId vertex_id = 0;
            vertex_id < graph()->number_of_vertices();
            ++vertex_id){
        instance.set_weight(vertex_id, graph()->weight(vertex_id));
        neighbors.clear();
        neighbors.add(vertex_id);
        for (auto it = graph()->neighbors_begin(vertex_id);
                it != graph()->neighbors_end(vertex_id);
                ++it) {
            neighbors.add(*it);
        }
        for (auto it = neighbors.out_begin(); it != neighbors.out_end(); ++it)
            if (*it > vertex_id)
                instance.add_edge(vertex_id, *it);
    }

    return instance;
}

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

void cliquesolver::init_display(
        const Instance& instance,
        optimizationtools::Info& info)
{
    const optimizationtools::AbstractGraph* graph = instance.graph();
    VertexId n = graph->number_of_vertices();
    EdgeId m = graph->number_of_edges();
    info.os()
        << "=====================================" << std::endl
        << "            Clique Solver            " << std::endl
        << "=====================================" << std::endl
        << std::endl
        << "Instance" << std::endl
        << "--------" << std::endl
        << "Number of vertices:              " << n << std::endl
        << "Number of edges:                 " << m << std::endl
        << "Density:                         " << (double)m * 2 / n / (n - 1) << std::endl
        << "Average degree:                  " << (double)graph->number_of_edges() * 2 / n << std::endl
        << "Maximum degree:                  " << graph->maximum_degree() << std::endl
        << std::endl;
}
