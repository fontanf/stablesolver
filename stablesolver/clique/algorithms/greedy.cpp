#include "stablesolver/clique/algorithms/greedy.hpp"

#include "stablesolver/clique/algorithm_formatter.hpp"

#include "optimizationtools/containers/doubly_indexed_map.hpp"

using namespace stablesolver::clique;

const Output stablesolver::clique::greedy_gwmin(
        const Instance& instance,
        const Parameters& parameters)
{
    Output output(instance);
    AlgorithmFormatter algorithm_formatter(parameters, output);
    algorithm_formatter.start("Greedy GWMIN");
    algorithm_formatter.print_header();

    const optimizationtools::AbstractGraph* graph = instance.graph();
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
    algorithm_formatter.update_solution(solution, "");

    algorithm_formatter.end();
    return output;
}

const Output stablesolver::clique::greedy_strong(
        const Instance& instance,
        const Parameters& parameters)
{
    Output output(instance);
    AlgorithmFormatter algorithm_formatter(parameters, output);
    algorithm_formatter.start("Strong greedy");
    algorithm_formatter.print_header();

    const optimizationtools::AbstractGraph* graph = instance.graph();
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
    algorithm_formatter.update_solution(solution, "");

    algorithm_formatter.end();
    return output;
}
