#include "cliquesolver/algorithms/greedy.hpp"

#include "optimizationtools/indexed_binary_heap.hpp"
#include "optimizationtools/doubly_indexed_map.hpp"

using namespace cliquesolver;

Output cliquesolver::greedy_gwmin(
        const Instance& instance,
        optimizationtools::Info info)
{
    VER(info, "*** greedy_gwmin ***" << std::endl);
    Output output(instance, info);
    Solution solution(instance);

    std::vector<double> vertices_values(instance.number_of_vertices(), 0);
    for (VertexId v = 0; v < instance.number_of_vertices(); ++v)
        vertices_values[v] = (double)instance.vertex(v).weight / (instance.number_of_vertices() - 1 - instance.degree(v) + 1);

    std::vector<VertexId> sorted_vertices(instance.number_of_vertices(), 0);
    std::iota(sorted_vertices.begin(), sorted_vertices.end(), 0);
    std::sort(sorted_vertices.begin(), sorted_vertices.end(),
            [&vertices_values](VertexId v1, VertexId v2) -> bool
        {
            return vertices_values[v1] > vertices_values[v2];
        });

    std::vector<VertexPos> available_vertices(instance.number_of_vertices(), 0);
    for (VertexId v: sorted_vertices) {
        if (available_vertices[v] < solution.number_of_vertices())
            continue;
        solution.add(v);
        for (const auto& edge: instance.vertex(v).edges)
            available_vertices[edge.v]++;
    }

    output.update_solution(solution, std::stringstream(), info);
    return output.algorithm_end(info);
}

Output cliquesolver::greedy_strong(
        const Instance& instance,
        optimizationtools::Info info)
{
    VER(info, "*** greedy_strong ***" << std::endl);
    Output output(instance, info);
    Solution solution(instance);

    optimizationtools::DoublyIndexedMap candidates(
            instance.number_of_vertices(), instance.number_of_vertices());
    for (VertexId v = 0; v < instance.number_of_vertices(); ++v)
        candidates.set(v, 0);
    while (candidates.number_of_elements(solution.number_of_vertices()) > 0) {
        VertexId v_best = -1;
        VertexPos score_best = -1;
        for (auto it_v = candidates.begin(solution.number_of_vertices());
                it_v != candidates.end(solution.number_of_vertices()); ++it_v) {
            VertexId v = *it_v;
            VertexPos score = 0;
            for (auto edge: instance.vertex(v).edges)
                if (candidates.contains(edge.v))
                    score += instance.vertex(edge.v).weight;
            if (v_best == -1 || score_best < score) {
                v_best = v;
                score_best = score;
            }
        }
        solution.add(v_best);
        for (auto edge: instance.vertex(v_best).edges)
            candidates.set(edge.v, candidates[edge.v] + 1);
    }

    output.update_solution(solution, std::stringstream(), info);
    return output.algorithm_end(info);
}

