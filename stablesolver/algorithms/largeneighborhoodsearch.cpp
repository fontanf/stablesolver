#include "stablesolver/algorithms/largeneighborhoodsearch.hpp"

#include "stablesolver/algorithms/greedy.hpp"

#include "optimizationtools/containers/indexed_set.hpp"
#include "optimizationtools/containers/indexed_binary_heap.hpp"

using namespace stablesolver;

LargeNeighborhoodSearchOutput& LargeNeighborhoodSearchOutput::algorithm_end(
        optimizationtools::Info& info)
{
    info.add_to_json("Algorithm", "Iterations", iterations);
    Output::algorithm_end(info);
    info.os() << "Iterations: " << iterations << std::endl;
    return *this;
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
        Instance& instance,
        LargeNeighborhoodSearchOptionalParameters parameters)
{
    init_display(instance, parameters.info);
    parameters.info.os()
        << "Algorithm" << std::endl
        << "---------" << std::endl
        << "Large Neighborhood Search" << std::endl
        << std::endl;

    //instance.fix_identical(parameters.info);
    //instance.fix_dominated(parameters.info);

    LargeNeighborhoodSearchOutput output(instance, parameters.info);
    Solution solution = greedy_gwmin(instance).solution;
    std::stringstream ss;
    ss << "initial solution";
    output.update_solution(solution, ss, parameters.info);

    // Initialize local search structures.
    std::vector<LargeNeighborhoodSearchVertex> vertices(instance.number_of_vertices());
    std::vector<Penalty> solution_penalties(instance.number_of_edges(), 1);
    for (auto it_v = solution.vertices().out_begin(); it_v != solution.vertices().out_end(); ++it_v) {
        VertexId v = *it_v;
        for (const auto& edge: instance.vertex(v).edges)
            if (solution.contains(edge.v))
                vertices[v].score += solution_penalties[edge.e];
    }
    optimizationtools::IndexedBinaryHeap<std::pair<double, Counter>> scores_out(instance.number_of_vertices());
    optimizationtools::IndexedBinaryHeap<std::pair<double, Counter>> scores_in(instance.number_of_vertices());
    for (auto it_v = solution.vertices().out_begin(); it_v != solution.vertices().out_end(); ++it_v) {
        VertexId v = *it_v;
        scores_out.update_key(v, {(double)vertices[v].score / instance.vertex(v).weight, 0});
    }

    optimizationtools::IndexedSet sets_in_to_update(instance.number_of_vertices());
    optimizationtools::IndexedSet sets_out_to_update(instance.number_of_vertices());
    Counter iterations_without_improvment = 0;
    for (output.iterations = 1; !parameters.info.needs_to_end(); ++output.iterations, ++iterations_without_improvment) {
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
        for (VertexPos s_tmp = 0; s_tmp < number_of_removed_vertices && !scores_out.empty(); ++s_tmp) {
            auto p = scores_out.top();
            scores_out.pop();
            VertexId v = p.first;
            solution.add(v);
            //std::cout << "add " << v
                //<< " p.second " << p.second.first
                //<< " score " << vertices[v].score
                //<< " weight " << instance.vertex(v).weight
                //<< " p " << solution.penalty()
                //<< " p_tmp " << p_tmp
                //<< std::endl;
            vertices[v].last_addition = output.iterations;
            sets_in_to_update.add(v);
            // Update scores.
            sets_out_to_update.clear();
            for (const auto& edge: instance.vertex(v).edges) {
                if (solution.contains(edge.v)) {
                    vertices[edge.v].score += solution_penalties[edge.e];
                    sets_in_to_update.add(edge.v);
                } else {
                    vertices[edge.v].score += solution_penalties[edge.e];
                    sets_out_to_update.add(edge.v);
                }
            }
            for (VertexId v2: sets_out_to_update)
                scores_out.update_key(v2, {(double)vertices[v2].score / instance.vertex(v2).weight, vertices[v2].last_removal});
        }
        for (VertexId v2: sets_in_to_update)
            scores_in.update_key(v2, {- (double)vertices[v2].score / instance.vertex(v2).weight, vertices[v2].last_removal});

        // Update penalties: we increment the penalty of each uncovered element.
        sets_in_to_update.clear();
        for (auto it = solution.conflicts().begin(); it != solution.conflicts().end(); ++it) {
            VertexId v1 = instance.edge(*it).v1;
            VertexId v2 = instance.edge(*it).v2;
            solution_penalties[*it]++;
            vertices[v1].score++;
            vertices[v2].score++;
            sets_in_to_update.add(v1);
            sets_in_to_update.add(v2);
        }
        for (VertexId v: sets_in_to_update)
            scores_in.update_key(v, {- (double)vertices[v].score / instance.vertex(v).weight, vertices[v].last_removal});

        // Remove vertices.
        sets_out_to_update.clear();
        while (!solution.feasible() && !scores_in.empty()) {
            auto p = scores_in.top();
            scores_in.pop();
            VertexId v = p.first;
            solution.remove(v);
            //std::cout << "remove " << v
                //<< " p.second " << p.second.first
                //<< " score " << vertices[v].score
                //<< " weight " << instance.vertex(v).weight
                //<< " p " << solution.penalty()
                //<< " p_tmp " << p_tmp
                //<< std::endl;
            assert(p.second.first < 0);
            vertices[v].last_removal = output.iterations;
            sets_out_to_update.add(v);
            // Update scores.
            sets_in_to_update.clear();
            for (const auto& edge: instance.vertex(v).edges) {
                if (solution.contains(edge.v)) {
                    vertices[edge.v].score -= solution_penalties[edge.e];
                    sets_in_to_update.add(edge.v);
                } else {
                    vertices[edge.v].score -= solution_penalties[edge.e];
                    sets_out_to_update.add(edge.v);
                }
            }

            // Remove redundant sets.
            for (const auto& edge: instance.vertex(v).edges) {
                if (!solution.contains(edge.v) && vertices[edge.v].score == 0) {
                    solution.add(edge.v);
                    //std::cout << "> add " << edge.v
                        //<< " score " << vertices[edge.v].score
                        //<< " weight " << instance.vertex(edge.v).weight
                        //<< " p " << solution.penalty()
                        //<< std::endl;
                    vertices[edge.v].last_addition = output.iterations;
                    sets_in_to_update.add(edge.v);
                    for (const auto& edge_2: instance.vertex(edge.v).edges) {
                        assert(!solution.contains(edge_2.v));
                        vertices[edge_2.v].score += solution_penalties[edge_2.e];
                        sets_out_to_update.add(edge_2.v);
                    }
                }
            }

            for (VertexId v2: sets_in_to_update)
                scores_in.update_key(v2, {- (double)vertices[v2].score / instance.vertex(v2).weight, vertices[v2].last_removal});
        }
        for (VertexId v2: sets_out_to_update) {
            if (!solution.contains(v2)) {
                scores_out.update_key(v2, {(double)vertices[v2].score / instance.vertex(v2).weight, vertices[v2].last_addition});
            } else {
                scores_out.update_key(v2, {-1, -1});
                assert(scores_out.top().first == v2);
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

    return output.algorithm_end(parameters.info);
}

