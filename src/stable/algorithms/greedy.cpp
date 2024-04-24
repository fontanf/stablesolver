#include "stablesolver/stable/algorithms/greedy.hpp"

#include "stablesolver/stable/algorithm_formatter.hpp"

#include "optimizationtools/containers/indexed_binary_heap.hpp"

using namespace stablesolver::stable;

const Output stablesolver::stable::greedy_gwmin(
        const Instance& instance,
        const GreedyParameters& parameters)
{
    Output output(instance);
    AlgorithmFormatter algorithm_formatter(parameters, output);
    algorithm_formatter.start("Greedy GWMIN");

    // Reduction.
    if (parameters.reduction_parameters.reduce)
        return solve_reduced_instance(greedy_gwmin, instance, parameters, algorithm_formatter, output);

    algorithm_formatter.print_header();

    Solution solution(instance);

    std::vector<double> vertices_values(instance.number_of_vertices(), 0);
    for (VertexId vertex_id = 0;
            vertex_id < instance.number_of_vertices();
            ++vertex_id) {
        vertices_values[vertex_id] = (double)instance.vertex(vertex_id).weight
            / (instance.degree(vertex_id) + 1);
    }

    std::vector<VertexId> sorted_vertices(instance.number_of_vertices(), 0);
    std::iota(sorted_vertices.begin(), sorted_vertices.end(), 0);
    std::sort(sorted_vertices.begin(), sorted_vertices.end(),
            [&vertices_values](VertexId vertex_id_1, VertexId vertex_id_2) -> bool
        {
            return vertices_values[vertex_id_1] > vertices_values[vertex_id_2];
        });

    std::vector<int8_t> available_vertices(instance.number_of_vertices(), 1);
    for (VertexId vertex_id: sorted_vertices) {
        if (!available_vertices[vertex_id])
            continue;
        solution.add(vertex_id);
        for (const auto& edge: instance.vertex(vertex_id).edges)
            available_vertices[edge.vertex_id] = 0;
    }
    algorithm_formatter.update_solution(solution, "");

    algorithm_formatter.end();
    return output;
}

const Output stablesolver::stable::greedy_gwmax(
        const Instance& instance,
        const GreedyParameters& parameters)
{
    Output output(instance);
    AlgorithmFormatter algorithm_formatter(parameters, output);
    algorithm_formatter.start("Greedy GWMAX");

    // Reduction.
    if (parameters.reduction_parameters.reduce)
        return solve_reduced_instance(greedy_gwmax, instance, parameters, algorithm_formatter, output);

    algorithm_formatter.print_header();

    auto f = [&instance](VertexId vertex_id)
    {
        VertexId d = instance.degree(vertex_id);
        double val = (d != 0)?
            (double)instance.vertex(vertex_id).weight / d / (d + 1):
            std::numeric_limits<double>::infinity();
        return std::pair<double, VertexId>{val, vertex_id};
    };
    optimizationtools::IndexedBinaryHeap<std::pair<double, VertexId>> heap(instance.number_of_vertices(), f);

    std::vector<uint8_t> removed_vertices(instance.number_of_vertices(), 0);
    for (;;) {
        auto p = heap.top();
        if (p.second.first == std::numeric_limits<double>::infinity())
            break;
        VertexId d = 0;
        for (const auto& vn: instance.vertex(p.first).edges)
            if (!removed_vertices[vn.vertex_id])
                d++;
        double val = (d != 0)?
            (double)instance.vertex(p.first).weight / d / (d + 1):
            std::numeric_limits<double>::infinity();
        if (val <= p.second.first + FFOT_TOL) {
            removed_vertices[p.first] = 1;
            heap.pop();
        } else {
            heap.update_key(p.first, {val, p.first});
        }
    }

    Solution solution(instance);
    for (VertexId vertex_id = 0;
            vertex_id < instance.number_of_vertices();
            ++vertex_id) {
        if (!removed_vertices[vertex_id])
            solution.add(vertex_id);
    }
    algorithm_formatter.update_solution(solution, "");

    algorithm_formatter.end();
    return output;
}

const Output stablesolver::stable::greedy_gwmin2(
        const Instance& instance,
        const GreedyParameters& parameters)
{
    Output output(instance);
    AlgorithmFormatter algorithm_formatter(parameters, output);
    algorithm_formatter.start("Greedy GWMIN2");

    // Reduction.
    if (parameters.reduction_parameters.reduce)
        return solve_reduced_instance(greedy_gwmin2, instance, parameters, algorithm_formatter, output);

    algorithm_formatter.print_header();

    Solution solution(instance);

    std::vector<double> vertices_values(instance.number_of_vertices(), 0);
    for (VertexId vertex_id = 0;
            vertex_id < instance.number_of_vertices();
            ++vertex_id) {
        Weight weight = 0;
        for (const auto& edge: instance.vertex(vertex_id).edges)
            weight += instance.vertex(edge.vertex_id).weight;
        vertices_values[vertex_id] = (weight != 0)?
            (double)instance.vertex(vertex_id).weight / weight:
            std::numeric_limits<double>::infinity();
    }

    std::vector<VertexId> sorted_vertices(instance.number_of_vertices(), 0);
    std::iota(sorted_vertices.begin(), sorted_vertices.end(), 0);
    std::sort(sorted_vertices.begin(), sorted_vertices.end(),
            [&vertices_values](VertexId vertex_id_1, VertexId vertex_id_2) -> bool
        {
            return vertices_values[vertex_id_1] > vertices_values[vertex_id_2];
        });

    std::vector<int8_t> available_vertices(instance.number_of_vertices(), 1);
    for (VertexId vertex_id: sorted_vertices) {
        if (!available_vertices[vertex_id])
            continue;
        solution.add(vertex_id);
        for (const auto& edge: instance.vertex(vertex_id).edges)
            available_vertices[edge.vertex_id] = 0;
    }
    algorithm_formatter.update_solution(solution, "");

    algorithm_formatter.end();
    return output;
}

const Output stablesolver::stable::greedy_strong(
        const Instance& instance,
        const GreedyParameters& parameters)
{
    Output output(instance);
    AlgorithmFormatter algorithm_formatter(parameters, output);
    algorithm_formatter.start("Strong greedy");

    // Reduction.
    if (parameters.reduction_parameters.reduce)
        return solve_reduced_instance(greedy_strong, instance, parameters, algorithm_formatter, output);

    algorithm_formatter.print_header();

    Solution solution(instance);

    optimizationtools::IndexedSet candidates(instance.number_of_vertices());
    candidates.fill();
    while (candidates.size() > 0) {
        VertexId vertex_id_best = -1;
        VertexPos score_best = -1;
        for (VertexId vertex_id: candidates) {
            VertexPos score = 0;
            for (auto edge: instance.vertex(vertex_id).edges)
                if (candidates.contains(edge.vertex_id))
                    score -= instance.vertex(edge.vertex_id).weight;
            if (vertex_id_best == -1 || score_best < score) {
                vertex_id_best = vertex_id;
                score_best = score;
            }
        }
        solution.add(vertex_id_best);
        candidates.remove(vertex_id_best);
        for (auto edge: instance.vertex(vertex_id_best).edges)
            candidates.remove(edge.vertex_id);
    }
    algorithm_formatter.update_solution(solution, "");

    algorithm_formatter.end();
    return output;
}
