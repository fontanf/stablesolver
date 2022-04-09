#include "cliquesolver/instance.hpp"

using namespace cliquesolver;

Instance::Instance(std::string instance_path, std::string format):
    graph_(std::unique_ptr<optimizationtools::AbstractGraph>(
                new optimizationtools::AdjacencyListGraph(instance_path, format))),
    adjacency_list_graph_(static_cast<optimizationtools::AdjacencyListGraph*>(graph_.get())) { }

Instance::Instance(VertexId number_of_vertices):
    graph_(std::unique_ptr<optimizationtools::AbstractGraph>(
                new optimizationtools::AdjacencyListGraph())),
    adjacency_list_graph_(static_cast<optimizationtools::AdjacencyListGraph*>(graph_.get()))
{
    for (VertexId v = 0; v < number_of_vertices; ++v)
        adjacency_list_graph_->add_vertex();
}

Instance::Instance(const optimizationtools::AbstractGraph& abstract_graph):
    graph_(std::unique_ptr<optimizationtools::AbstractGraph>(abstract_graph.clone())) { }

Instance Instance::complementary()
{
    Instance instance(graph()->number_of_vertices());
    optimizationtools::IndexedSet neighbors(graph()->number_of_vertices());

    for (VertexId v = 0; v < graph()->number_of_vertices(); ++v){
        instance.set_weight(v, graph()->weight(v));
        neighbors.clear();
        neighbors.add(v);
        for (auto it = graph()->neighbors_begin(v);
                it != graph()->neighbors_end(v); ++it) {
            neighbors.add(*it);
        }
        for (auto it = neighbors.out_begin(); it != neighbors.out_end(); ++it)
            if (*it > v)
                instance.add_edge(v, *it);
    }

    return instance;
}

Weight Instance::update_core(
        optimizationtools::IndexedSet& relevant_vertices,
        Weight weight) const
{
    std::vector<Weight> best_values(graph()->number_of_vertices(), 0);
    std::vector<VertexId> vertex_queue;
    for (VertexId v: relevant_vertices) {
        best_values[v] = graph()->weight(v);
        for (auto it = graph()->neighbors_begin(v);
                it != graph()->neighbors_end(v); ++it) {
            if (relevant_vertices.contains(*it))
                best_values[v] += graph()->weight(*it);
        }
        if (best_values[v] <= weight)
            vertex_queue.push_back(v);
    }
    while (!vertex_queue.empty()) {
        VertexId v = vertex_queue.back();
        relevant_vertices.remove(v);
        vertex_queue.pop_back();
        for (auto it = graph()->neighbors_begin(v);
                it != graph()->neighbors_end(v); ++it) {
            if (best_values[*it] <= weight)
                continue;
            best_values[*it] -= graph()->weight(v);
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
    FFOT_VER(info,
               "=====================================" << std::endl
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
            << std::endl);
}
