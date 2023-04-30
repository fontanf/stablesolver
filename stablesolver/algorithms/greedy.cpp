#include "stablesolver/algorithms/greedy.hpp"

#include "optimizationtools/containers/indexed_binary_heap.hpp"

using namespace stablesolver;

Output stablesolver::greedy_gwmin(
        const Instance& original_instance,
        GreedyOptionalParameters parameters)
{
    init_display(original_instance, parameters.info);
    parameters.info.os()
        << "Algorithm" << std::endl
        << "---------" << std::endl
        << "Greedy GWMIN" << std::endl
        << std::endl;

    // Reduction.
    std::unique_ptr<Instance> reduced_instance = nullptr;
    if (parameters.reduction_parameters.reduce) {
        reduced_instance = std::unique_ptr<Instance>(
                new Instance(
                    original_instance.reduce(
                        parameters.reduction_parameters)));
        parameters.info.os()
            << "Reduced instance" << std::endl
            << "----------------" << std::endl;
        reduced_instance->print(parameters.info.os(), parameters.info.verbosity_level());
        parameters.info.os() << std::endl;
    }
    const Instance& instance = (reduced_instance == nullptr)? original_instance: *reduced_instance;

    Output output(original_instance, parameters.info);

    // Update upper bound from reduction.
    if (reduced_instance != nullptr) {
        output.update_upper_bound(
                reduced_instance->total_weight()
                + reduced_instance->unreduction_info().extra_weight,
                std::stringstream("reduction"),
                parameters.info);
    }

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

    output.update_solution(solution, std::stringstream(), parameters.info);
    return output.algorithm_end(parameters.info);
}

Output stablesolver::greedy_gwmax(
        const Instance& original_instance,
        GreedyOptionalParameters parameters)
{
    init_display(original_instance, parameters.info);
    parameters.info.os()
        << "Algorithm" << std::endl
        << "---------" << std::endl
        << "Greedy GWMAX" << std::endl
        << std::endl;

    // Reduction.
    std::unique_ptr<Instance> reduced_instance = nullptr;
    if (parameters.reduction_parameters.reduce) {
        reduced_instance = std::unique_ptr<Instance>(
                new Instance(
                    original_instance.reduce(
                        parameters.reduction_parameters)));
        parameters.info.os()
            << "Reduced instance" << std::endl
            << "----------------" << std::endl;
        reduced_instance->print(parameters.info.os(), parameters.info.verbosity_level());
        parameters.info.os() << std::endl;
    }
    const Instance& instance = (reduced_instance == nullptr)? original_instance: *reduced_instance;

    Output output(original_instance, parameters.info);

    // Update upper bound from reduction.
    if (reduced_instance != nullptr) {
        output.update_upper_bound(
                reduced_instance->total_weight()
                + reduced_instance->unreduction_info().extra_weight,
                std::stringstream("reduction"),
                parameters.info);
    }

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

    output.update_solution(solution, std::stringstream(), parameters.info);
    return output.algorithm_end(parameters.info);
}

Output stablesolver::greedy_gwmin2(
        const Instance& original_instance,
        GreedyOptionalParameters parameters)
{
    init_display(original_instance, parameters.info);
    parameters.info.os()
        << "Algorithm" << std::endl
        << "---------" << std::endl
        << "Greedy GWMIN2" << std::endl
        << std::endl;

    // Reduction.
    std::unique_ptr<Instance> reduced_instance = nullptr;
    if (parameters.reduction_parameters.reduce) {
        reduced_instance = std::unique_ptr<Instance>(
                new Instance(
                    original_instance.reduce(
                        parameters.reduction_parameters)));
        parameters.info.os()
            << "Reduced instance" << std::endl
            << "----------------" << std::endl;
        reduced_instance->print(parameters.info.os(), parameters.info.verbosity_level());
        parameters.info.os() << std::endl;
    }
    const Instance& instance = (reduced_instance == nullptr)? original_instance: *reduced_instance;

    Output output(original_instance, parameters.info);

    // Update upper bound from reduction.
    if (reduced_instance != nullptr) {
        output.update_upper_bound(
                reduced_instance->total_weight()
                + reduced_instance->unreduction_info().extra_weight,
                std::stringstream("reduction"),
                parameters.info);
    }

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

    output.update_solution(solution, std::stringstream(), parameters.info);
    return output.algorithm_end(parameters.info);
}

Output stablesolver::greedy_strong(
        const Instance& original_instance,
        GreedyOptionalParameters parameters)
{
    init_display(original_instance, parameters.info);
    parameters.info.os()
        << "Algorithm" << std::endl
        << "---------" << std::endl
        << "Strong Greedy" << std::endl
        << std::endl;

    // Reduction.
    std::unique_ptr<Instance> reduced_instance = nullptr;
    if (parameters.reduction_parameters.reduce) {
        reduced_instance = std::unique_ptr<Instance>(
                new Instance(
                    original_instance.reduce(
                        parameters.reduction_parameters)));
        parameters.info.os()
            << "Reduced instance" << std::endl
            << "----------------" << std::endl;
        reduced_instance->print(parameters.info.os(), parameters.info.verbosity_level());
        parameters.info.os() << std::endl;
    }
    const Instance& instance = (reduced_instance == nullptr)? original_instance: *reduced_instance;

    Output output(original_instance, parameters.info);

    // Update upper bound from reduction.
    if (reduced_instance != nullptr) {
        output.update_upper_bound(
                reduced_instance->total_weight()
                + reduced_instance->unreduction_info().extra_weight,
                std::stringstream("reduction"),
                parameters.info);
    }

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

    output.update_solution(solution, std::stringstream(), parameters.info);
    return output.algorithm_end(parameters.info);
}

