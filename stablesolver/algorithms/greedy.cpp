#include "stablesolver/algorithms/greedy.hpp"

#include "optimizationtools/containers/indexed_binary_heap.hpp"

using namespace stablesolver;

Output stablesolver::greedy_gwmin(
        const Instance& instance_original,
        optimizationtools::Info info)
{
    init_display(instance_original, info);
    info.os()
        << "Algorithm" << std::endl
        << "---------" << std::endl
        << "Greedy GWMIN" << std::endl
        << std::endl;

    const Instance& instance = (instance_original.reduced_instance() == nullptr)?  instance_original: *instance_original.reduced_instance();
    Output output(instance_original, info);
    Solution solution(instance);

    std::vector<double> vertices_values(instance.number_of_vertices(), 0);
    for (VertexId v = 0; v < instance.number_of_vertices(); ++v)
        vertices_values[v] = (double)instance.vertex(v).weight / (instance.degree(v) + 1);

    std::vector<VertexId> sorted_vertices(instance.number_of_vertices(), 0);
    std::iota(sorted_vertices.begin(), sorted_vertices.end(), 0);
    std::sort(sorted_vertices.begin(), sorted_vertices.end(),
            [&vertices_values](VertexId v1, VertexId v2) -> bool
        {
            return vertices_values[v1] > vertices_values[v2];
        });

    std::vector<int8_t> available_vertices(instance.number_of_vertices(), 1);
    for (VertexId v: sorted_vertices) {
        if (!available_vertices[v])
            continue;
        solution.add(v);
        for (const auto& edge: instance.vertex(v).edges)
            available_vertices[edge.v] = 0;
    }

    output.update_solution(solution, std::stringstream(), info);
    return output.algorithm_end(info);
}

Output stablesolver::greedy_gwmax(
        const Instance& instance_original,
        optimizationtools::Info info)
{
    init_display(instance_original, info);
    info.os()
        << "Algorithm" << std::endl
        << "---------" << std::endl
        << "Greedy GWMAX" << std::endl
        << std::endl;

    Output output(instance_original, info);
    const Instance& instance = (instance_original.reduced_instance() == nullptr)?  instance_original: *instance_original.reduced_instance();

    auto f = [&instance](VertexId v)
    {
        VertexId d = instance.degree(v);
        double val = (d != 0)?
            (double)instance.vertex(v).weight / d / (d + 1):
            std::numeric_limits<double>::infinity();
        return std::pair<double, VertexId>{val, v};
    };
    optimizationtools::IndexedBinaryHeap<std::pair<double, VertexId>> heap(instance.number_of_vertices(), f);

    std::vector<uint8_t> removed_vertices(instance.number_of_vertices(), 0);
    for (;;) {
        auto p = heap.top();
        if (p.second.first == std::numeric_limits<double>::infinity())
            break;
        VertexId d = 0;
        for (const auto& vn: instance.vertex(p.first).edges)
            if (!removed_vertices[vn.v])
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
    for (VertexId v = 0; v < instance.number_of_vertices(); ++v)
        if (!removed_vertices[v])
            solution.add(v);

    output.update_solution(solution, std::stringstream(), info);
    return output.algorithm_end(info);
}

Output stablesolver::greedy_gwmin2(
        const Instance& instance_original,
        optimizationtools::Info info)
{
    init_display(instance_original, info);
    info.os()
        << "Algorithm" << std::endl
        << "---------" << std::endl
        << "Greedy GWMIN2" << std::endl
        << std::endl;

    Output output(instance_original, info);
    const Instance& instance = (instance_original.reduced_instance() == nullptr)?  instance_original: *instance_original.reduced_instance();
    Solution solution(instance);

    std::vector<double> vertices_values(instance.number_of_vertices(), 0);
    for (VertexId v = 0; v < instance.number_of_vertices(); ++v) {
        Weight w = 0;
        for (const auto& edge: instance.vertex(v).edges)
            w += instance.vertex(edge.v).weight;
        vertices_values[v] = (w != 0)?
            (double)instance.vertex(v).weight / w:
            std::numeric_limits<double>::infinity();
    }

    std::vector<VertexId> sorted_vertices(instance.number_of_vertices(), 0);
    std::iota(sorted_vertices.begin(), sorted_vertices.end(), 0);
    std::sort(sorted_vertices.begin(), sorted_vertices.end(),
            [&vertices_values](VertexId v1, VertexId v2) -> bool
        {
            return vertices_values[v1] > vertices_values[v2];
        });

    std::vector<int8_t> available_vertices(instance.number_of_vertices(), 1);
    for (VertexId v: sorted_vertices) {
        if (!available_vertices[v])
            continue;
        solution.add(v);
        for (const auto& edge: instance.vertex(v).edges)
            available_vertices[edge.v] = 0;
    }

    output.update_solution(solution, std::stringstream(), info);
    return output.algorithm_end(info);
}

Output stablesolver::greedy_strong(
        const Instance& instance_original,
        optimizationtools::Info info)
{
    init_display(instance_original, info);
    info.os()
        << "Algorithm" << std::endl
        << "---------" << std::endl
        << "Strong Greedy" << std::endl
        << std::endl;

    Output output(instance_original, info);
    const Instance& instance = (instance_original.reduced_instance() == nullptr)?  instance_original: *instance_original.reduced_instance();
    Solution solution(instance);

    optimizationtools::IndexedSet candidates(instance.number_of_vertices());
    candidates.fill();
    while (candidates.size() > 0) {
        VertexId v_best = -1;
        VertexPos score_best = -1;
        for (VertexId v: candidates) {
            VertexPos score = 0;
            for (auto edge: instance.vertex(v).edges)
                if (candidates.contains(edge.v))
                    score -= instance.vertex(edge.v).weight;
            if (v_best == -1 || score_best < score) {
                v_best = v;
                score_best = score;
            }
        }
        solution.add(v_best);
        candidates.remove(v_best);
        for (auto edge: instance.vertex(v_best).edges)
            candidates.remove(edge.v);
    }

    output.update_solution(solution, std::stringstream(), info);
    return output.algorithm_end(info);
}

