#include "cliquesolver/algorithms/greedy.hpp"

#include "optimizationtools/indexed_binary_heap.hpp"

using namespace cliquesolver;

Output cliquesolver::greedy_gwmin(const Instance& instance, Info info)
{
    VER(info, "*** greedy_gwmin ***" << std::endl);
    Output output(instance, info);
    Solution solution(instance);

    std::vector<double> vertices_values(instance.vertex_number(), 0);
    for (VertexId v = 0; v < instance.vertex_number(); ++v)
        vertices_values[v] = (double)instance.vertex(v).weight / (instance.vertex_number() - 1 - instance.degree(v) + 1);

    std::vector<VertexId> sorted_vertices(instance.vertex_number(), 0);
    std::iota(sorted_vertices.begin(), sorted_vertices.end(), 0);
    std::sort(sorted_vertices.begin(), sorted_vertices.end(),
            [&vertices_values](VertexId v1, VertexId v2) -> bool
        {
            return vertices_values[v1] > vertices_values[v2];
        });

    std::vector<VertexPos> available_vertices(instance.vertex_number(), 0);
    for (VertexId v: sorted_vertices) {
        if (available_vertices[v] < solution.vertex_number())
            continue;
        solution.add(v);
        for (const auto& edge: instance.vertex(v).edges)
            available_vertices[edge.v]++;
    }

    output.update_solution(solution, std::stringstream(), info);
    return output.algorithm_end(info);
}

Output cliquesolver::greedy_gwmax(const Instance& instance, Info info)
{
    VER(info, "*** greedy_gwmax ***" << std::endl);
    Output output(instance, info);

    auto f = [&instance](VertexId v)
    {
        VertexId d = instance.degree(v);
        double val = (d != 0)?
            (double)instance.vertex(v).weight / d / (d + 1):
            std::numeric_limits<double>::infinity();
        return std::pair<double, VertexId>{val, v};
    };
    optimizationtools::IndexedBinaryHeap<std::pair<double, VertexId>> heap(instance.vertex_number(), f);

    std::vector<uint8_t> removed_vertices(instance.vertex_number(), 0);
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
        if (val <= p.second.first + TOL) {
            removed_vertices[p.first] = 1;
            heap.pop();
        } else {
            heap.update_key(p.first, {val, p.first});
        }
    }

    Solution solution(instance);
    for (VertexId v = 0; v < instance.vertex_number(); ++v)
        if (!removed_vertices[v])
            solution.add(v);

    output.update_solution(solution, std::stringstream(), info);
    return output.algorithm_end(info);
}

Output cliquesolver::greedy_gwmin2(const Instance& instance, Info info)
{
    VER(info, "*** greedy_gwmin2 ***" << std::endl);
    Output output(instance, info);
    Solution solution(instance);

    std::vector<double> vertices_values(instance.vertex_number(), 0);
    for (VertexId v = 0; v < instance.vertex_number(); ++v) {
        Weight w = 0;
        for (const auto& edge: instance.vertex(v).edges)
            w += instance.vertex(edge.v).weight;
        vertices_values[v] = (w != 0)?
            (double)instance.vertex(v).weight / w:
            std::numeric_limits<double>::infinity();
    }

    std::vector<VertexId> sorted_vertices(instance.vertex_number(), 0);
    std::iota(sorted_vertices.begin(), sorted_vertices.end(), 0);
    std::sort(sorted_vertices.begin(), sorted_vertices.end(),
            [&vertices_values](VertexId v1, VertexId v2) -> bool
        {
            return vertices_values[v1] > vertices_values[v2];
        });

    std::vector<int8_t> available_vertices(instance.vertex_number(), 1);
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

