#include "cliquesolver/algorithms/greedy.hpp"

#include "optimizationtools/containers/indexed_binary_heap.hpp"
#include "optimizationtools/containers/doubly_indexed_map.hpp"

using namespace cliquesolver;

Output cliquesolver::greedy_gwmin(
        const Instance& instance,
        optimizationtools::Info info)
{
    cliquesolver::init_display(instance, info);
    info.os()
        << "Algorithm" << std::endl
        << "---------" << std::endl
        << "Greedy GWMIN" << std::endl
        << std::endl;

    const optimizationtools::AbstractGraph* graph = instance.graph();
    Output output(instance, info);
    Solution solution(instance);

    std::vector<double> vertices_values(graph->number_of_vertices(), 0);
    for (VertexId vertex_id = 0;
            vertex_id < graph->number_of_vertices();
            ++vertex_id) {
        vertices_values[vertex_id] = (double)graph->weight(vertex_id)
            / (graph->number_of_vertices() - 1 - graph->degree(vertex_id) + 1);
    }

    std::vector<VertexId> sorted_vertices(graph->number_of_vertices(), 0);
    std::iota(sorted_vertices.begin(), sorted_vertices.end(), 0);
    std::sort(sorted_vertices.begin(), sorted_vertices.end(),
            [&vertices_values](VertexId vertex_id_1, VertexId vertex_id_2) -> bool
        {
            return vertices_values[vertex_id_1] > vertices_values[vertex_id_2];
        });

    std::vector<VertexPos> available_vertices(graph->number_of_vertices(), 0);
    for (VertexId vertex_id: sorted_vertices) {
        if (available_vertices[vertex_id] < solution.number_of_vertices())
            continue;
        solution.add(vertex_id);
        for (auto it = graph->neighbors_begin(vertex_id);
                it != graph->neighbors_end(vertex_id);
                ++it) {
            available_vertices[*it]++;
        }
    }

    output.update_solution(solution, std::stringstream(), info);
    return output.algorithm_end(info);
}

Output cliquesolver::greedy_strong(
        const Instance& instance,
        optimizationtools::Info info)
{
    cliquesolver::init_display(instance, info);
    info.os()
        << "Algorithm" << std::endl
        << "---------" << std::endl
        << "Strong greedy" << std::endl
        << std::endl;

    const optimizationtools::AbstractGraph* graph = instance.graph();
    Output output(instance, info);
    Solution solution(instance);

    optimizationtools::DoublyIndexedMap candidates(
            graph->number_of_vertices(), graph->number_of_vertices());
    for (VertexId vertex_id = 0;
            vertex_id < graph->number_of_vertices();
            ++vertex_id)
        candidates.set(vertex_id, 0);
    while (candidates.number_of_elements(solution.number_of_vertices()) > 0) {
        VertexId vertex_id_best = -1;
        VertexPos score_best = -1;
        for (auto it_v = candidates.begin(solution.number_of_vertices());
                it_v != candidates.end(solution.number_of_vertices());
                ++it_v) {
            VertexId vertex_id = *it_v;
            VertexPos score = 0;
            for (auto it = graph->neighbors_begin(vertex_id);
                    it != graph->neighbors_end(vertex_id);
                    ++it) {
                if (candidates.contains(*it))
                    score += graph->weight(*it);
            }
            if (vertex_id_best == -1 || score_best < score) {
                vertex_id_best = vertex_id;
                score_best = score;
            }
        }
        solution.add(vertex_id_best);
        for (auto it = graph->neighbors_begin(vertex_id_best);
                it != graph->neighbors_end(vertex_id_best);
                ++it) {
            candidates.set(*it, candidates[*it] + 1);
        }
    }

    output.update_solution(solution, std::stringstream(), info);
    return output.algorithm_end(info);
}

