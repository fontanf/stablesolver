#include "stablesolver/algorithms/largeneighborhoodsearch.hpp"

#include "stablesolver/algorithms/greedy.hpp"

#include "optimizationtools/containers/indexed_set.hpp"
#include "optimizationtools/containers/indexed_binary_heap.hpp"

using namespace stablesolver;

void LargeNeighborhoodSearchOutput::print_statistics(
        optimizationtools::Info& info) const
{
    if (info.verbosity_level() >= 1) {
        info.os()
            << "Number of iterations:         " << iterations << std::endl
            ;
    }
    info.add_to_json("Algorithm", "NumberOfIterations", iterations);
}

struct LargeNeighborhoodSearchVertex
{
    Counter timestamp     = -1;
    Counter last_addition = -1;
    Counter last_removal  = -1;
    Counter iterations    = 0;
    Weight  score         = 0;
};

LargeNeighborhoodSearchOutput stablesolver::largeneighborhoodsearch(
        Instance& original_instance,
        LargeNeighborhoodSearchOptionalParameters parameters)
{
    init_display(original_instance, parameters.info);
    parameters.info.os()
        << "Algorithm" << std::endl
        << "---------" << std::endl
        << "Large Neighborhood Search" << std::endl
        << std::endl;

    //instance.fix_identical(parameters.info);
    //instance.fix_dominated(parameters.info);

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

    LargeNeighborhoodSearchOutput output(original_instance, parameters.info);

    // Update upper bound from reduction.
    if (reduced_instance != nullptr) {
        output.update_bound(
                reduced_instance->total_weight()
                + reduced_instance->unreduction_info().extra_weight,
                std::stringstream("reduction"),
                parameters.info);
    }

    Solution solution = greedy_gwmin(instance).solution;
    std::stringstream ss;
    ss << "initial solution";
    output.update_solution(solution, ss, parameters.info);

    // Initialize local search structures.
    std::vector<LargeNeighborhoodSearchVertex> vertices(instance.number_of_vertices());
    std::vector<Penalty> solution_penalties(instance.number_of_edges(), 1);
    for (auto it_v = solution.vertices().out_begin();
            it_v != solution.vertices().out_end();
            ++it_v) {
        VertexId vertex_id = *it_v;
        for (const auto& edge: instance.vertex(vertex_id).edges)
            if (solution.contains(edge.vertex_id))
                vertices[vertex_id].score += solution_penalties[edge.edge_id];
    }
    optimizationtools::IndexedBinaryHeap<std::pair<double, Counter>> scores_out(instance.number_of_vertices());
    optimizationtools::IndexedBinaryHeap<std::pair<double, Counter>> scores_in(instance.number_of_vertices());
    for (auto it_v = solution.vertices().out_begin();
            it_v != solution.vertices().out_end();
            ++it_v) {
        VertexId vertex_id = *it_v;
        scores_out.update_key(
                vertex_id,
                {(double)vertices[vertex_id].score / instance.vertex(vertex_id).weight, 0});
    }

    optimizationtools::IndexedSet sets_in_to_update(instance.number_of_vertices());
    optimizationtools::IndexedSet sets_out_to_update(instance.number_of_vertices());
    Counter iterations_without_improvment = 0;
    for (output.iterations = 1;
            !parameters.info.needs_to_end();
            ++output.iterations, ++iterations_without_improvment) {

        // Check stop criteria.
        if (parameters.maximum_number_of_iterations != -1
                && output.iterations > parameters.maximum_number_of_iterations)
            break;
        if (parameters.maximum_number_of_iterations_without_improvement != -1
                && iterations_without_improvment > parameters.maximum_number_of_iterations_without_improvement)
            break;
        //std::cout
            //<< "weight " << solution.weight()
            //<< " v " << solution.number_of_vertices()
            //<< " f " << solution.feasible()
            //<< std::endl;

        // Add vertices.
        VertexPos number_of_removed_vertices = sqrt(instance.number_of_vertices() - solution.number_of_vertices());
        //std::cout << "number_of_removed_vertices " << removed_number_of_vertices << std::endl;
        sets_in_to_update.clear();
        for (VertexPos s_tmp = 0;
                s_tmp < number_of_removed_vertices && !scores_out.empty();
                ++s_tmp) {
            auto p = scores_out.top();
            scores_out.pop();
            VertexId vertex_id = p.first;
            solution.add(vertex_id);
            //std::cout << "add " << v
                //<< " p.second " << p.second.first
                //<< " score " << vertices[v].score
                //<< " weight " << instance.vertex(v).weight
                //<< " p " << solution.penalty()
                //<< " p_tmp " << p_tmp
                //<< std::endl;
            vertices[vertex_id].last_addition = output.iterations;
            sets_in_to_update.add(vertex_id);
            // Update scores.
            sets_out_to_update.clear();
            for (const auto& edge: instance.vertex(vertex_id).edges) {
                if (solution.contains(edge.vertex_id)) {
                    vertices[edge.vertex_id].score += solution_penalties[edge.edge_id];
                    sets_in_to_update.add(edge.vertex_id);
                } else {
                    vertices[edge.vertex_id].score += solution_penalties[edge.edge_id];
                    sets_out_to_update.add(edge.vertex_id);
                }
            }
            for (VertexId vertex_id_2: sets_out_to_update)
                scores_out.update_key(vertex_id_2, {(double)vertices[vertex_id_2].score / instance.vertex(vertex_id_2).weight, vertices[vertex_id_2].last_removal});
        }
        for (VertexId vertex_id_2: sets_in_to_update)
            scores_in.update_key(vertex_id_2, {- (double)vertices[vertex_id_2].score / instance.vertex(vertex_id_2).weight, vertices[vertex_id_2].last_removal});

        // Update penalties: we increment the penalty of each uncovered element.
        sets_in_to_update.clear();
        for (auto it = solution.conflicts().begin();
                it != solution.conflicts().end();
                ++it) {
            VertexId vertex_id_1 = instance.edge(*it).vertex_id_1;
            VertexId vertex_id_2 = instance.edge(*it).vertex_id_2;
            solution_penalties[*it]++;
            vertices[vertex_id_1].score++;
            vertices[vertex_id_2].score++;
            sets_in_to_update.add(vertex_id_1);
            sets_in_to_update.add(vertex_id_2);
        }
        for (VertexId vertex_id: sets_in_to_update)
            scores_in.update_key(
                    vertex_id,
                    {- (double)vertices[vertex_id].score / instance.vertex(vertex_id).weight, vertices[vertex_id].last_removal});

        // Remove vertices.
        sets_out_to_update.clear();
        while (!solution.feasible() && !scores_in.empty()) {
            auto p = scores_in.top();
            scores_in.pop();
            VertexId vertex_id = p.first;
            solution.remove(vertex_id);
            //std::cout << "remove " << v
                //<< " p.second " << p.second.first
                //<< " score " << vertices[v].score
                //<< " weight " << instance.vertex(v).weight
                //<< " p " << solution.penalty()
                //<< " p_tmp " << p_tmp
                //<< std::endl;
            assert(p.second.first < 0);
            vertices[vertex_id].last_removal = output.iterations;
            sets_out_to_update.add(vertex_id);
            // Update scores.
            sets_in_to_update.clear();
            for (const auto& edge: instance.vertex(vertex_id).edges) {
                if (solution.contains(edge.vertex_id)) {
                    vertices[edge.vertex_id].score -= solution_penalties[edge.edge_id];
                    sets_in_to_update.add(edge.vertex_id);
                } else {
                    vertices[edge.vertex_id].score -= solution_penalties[edge.edge_id];
                    sets_out_to_update.add(edge.vertex_id);
                }
            }

            // Remove redundant sets.
            for (const auto& edge: instance.vertex(vertex_id).edges) {
                if (!solution.contains(edge.vertex_id)
                        && vertices[edge.vertex_id].score == 0) {
                    solution.add(edge.vertex_id);
                    //std::cout << "> add " << edge.v
                        //<< " score " << vertices[edge.v].score
                        //<< " weight " << instance.vertex(edge.v).weight
                        //<< " p " << solution.penalty()
                        //<< std::endl;
                    vertices[edge.vertex_id].last_addition = output.iterations;
                    sets_in_to_update.add(edge.vertex_id);
                    for (const auto& edge_2: instance.vertex(edge.vertex_id).edges) {
                        assert(!solution.contains(edge_2.vertex_id));
                        vertices[edge_2.vertex_id].score += solution_penalties[edge_2.edge_id];
                        sets_out_to_update.add(edge_2.vertex_id);
                    }
                }
            }

            for (VertexId vertex_id_2: sets_in_to_update) {
                scores_in.update_key(
                        vertex_id_2,
                        {- (double)vertices[vertex_id_2].score / instance.vertex(vertex_id_2).weight, vertices[vertex_id_2].last_removal});
            }
        }
        for (VertexId vertex_id_2: sets_out_to_update) {
            if (!solution.contains(vertex_id_2)) {
                scores_out.update_key(
                        vertex_id_2,
                        {(double)vertices[vertex_id_2].score / instance.vertex(vertex_id_2).weight, vertices[vertex_id_2].last_addition});
            } else {
                scores_out.update_key(vertex_id_2, {-1, -1});
                assert(scores_out.top().first == vertex_id_2);
                scores_out.pop();
            }
        }

        // Update best solution.
        //std::cout << "weight " << solution.weight() << std::endl;
        if (output.solution.weight() < solution.weight()){
            std::stringstream ss;
            ss << "iteration " << output.iterations;
            output.update_solution(solution, ss, parameters.info);
            iterations_without_improvment = 0;
        }
    }

    output.algorithm_end(parameters.info);
    return output;
}

