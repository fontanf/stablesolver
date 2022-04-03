#include "cliquesolver/algorithms/greedy.hpp"

#include "optimizationtools/containers/indexed_binary_heap.hpp"
#include "optimizationtools/containers/doubly_indexed_map.hpp"

using namespace cliquesolver;

Output cliquesolver::greedy_gwmin(
        const Instance& instance,
        optimizationtools::Info info)
{
    cliquesolver::init_display(instance, info);
    FFOT_VER(info,
               "Algorithm" << std::endl
            << "---------" << std::endl
            << "Greedy GWMIN" << std::endl
            << std::endl);

    const optimizationtools::AbstractGraph* graph = instance.graph();
    Output output(instance, info);
    Solution solution(instance);

    std::vector<double> vertices_values(graph->number_of_vertices(), 0);
    for (VertexId v = 0; v < graph->number_of_vertices(); ++v) {
        vertices_values[v] = (double)graph->weight(v)
            / (graph->number_of_vertices() - 1 - graph->degree(v) + 1);
    }

    std::vector<VertexId> sorted_vertices(graph->number_of_vertices(), 0);
    std::iota(sorted_vertices.begin(), sorted_vertices.end(), 0);
    std::sort(sorted_vertices.begin(), sorted_vertices.end(),
            [&vertices_values](VertexId v1, VertexId v2) -> bool
        {
            return vertices_values[v1] > vertices_values[v2];
        });

    std::vector<VertexPos> available_vertices(graph->number_of_vertices(), 0);
    for (VertexId v: sorted_vertices) {
        if (available_vertices[v] < solution.number_of_vertices())
            continue;
        solution.add(v);
        for (auto it = graph->neighbors_begin(v);
                it != graph->neighbors_end(v); ++it) {
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
    FFOT_VER(info,
               "Algorithm" << std::endl
            << "---------" << std::endl
            << "Strong Greedy" << std::endl
            << std::endl);

    const optimizationtools::AbstractGraph* graph = instance.graph();
    Output output(instance, info);
    Solution solution(instance);

    optimizationtools::DoublyIndexedMap candidates(
            graph->number_of_vertices(), graph->number_of_vertices());
    for (VertexId v = 0; v < graph->number_of_vertices(); ++v)
        candidates.set(v, 0);
    while (candidates.number_of_elements(solution.number_of_vertices()) > 0) {
        VertexId v_best = -1;
        VertexPos score_best = -1;
        for (auto it_v = candidates.begin(solution.number_of_vertices());
                it_v != candidates.end(solution.number_of_vertices()); ++it_v) {
            VertexId v = *it_v;
            VertexPos score = 0;
            for (auto it = graph->neighbors_begin(v);
                    it != graph->neighbors_end(v); ++it) {
                if (candidates.contains(*it))
                    score += graph->weight(*it);
            }
            if (v_best == -1 || score_best < score) {
                v_best = v;
                score_best = score;
            }
        }
        solution.add(v_best);
        for (auto it = graph->neighbors_begin(v_best);
                it != graph->neighbors_end(v_best); ++it) {
            candidates.set(*it, candidates[*it] + 1);
        }
    }

    output.update_solution(solution, std::stringstream(), info);
    return output.algorithm_end(info);
}

