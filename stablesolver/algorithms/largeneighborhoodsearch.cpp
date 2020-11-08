#include "stablesolver/algorithms/largeneighborhoodsearch.hpp"

#include "stablesolver/algorithms/greedy.hpp"

#include "optimizationtools/indexed_set.hpp"
#include "optimizationtools/indexed_binary_heap.hpp"

using namespace stablesolver;

LargeNeighborhoodSearchOutput& LargeNeighborhoodSearchOutput::algorithm_end(Info& info)
{
    PUT(info, "Algorithm", "Iterations", iterations);
    Output::algorithm_end(info);
    VER(info, "Iterations: " << iterations << std::endl);
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
    VER(parameters.info, "*** largeneighborhoodsearch ***" << std::endl);

    //instance.fix_identical(parameters.info);
    //instance.fix_dominated(parameters.info);

    LargeNeighborhoodSearchOutput output(instance, parameters.info);
    Solution solution = greedy_gwmin(instance).solution;
    std::stringstream ss;
    ss << "initial solution";
    output.update_solution(solution, ss, parameters.info);

    // Initialize local search structures.
    std::vector<LargeNeighborhoodSearchVertex> vertices(instance.vertex_number());
    for (auto it_v = solution.vertices().out_begin(); it_v != solution.vertices().out_end(); ++it_v) {
        VertexId v = *it_v;
        for (const auto& edge: instance.vertex(v).edges)
            if (solution.contains(edge.v))
                vertices[v].score += solution.penalty(edge.e);
    }
    optimizationtools::IndexedBinaryHeap<std::pair<double, Counter>> scores_out(instance.vertex_number());
    optimizationtools::IndexedBinaryHeap<std::pair<double, Counter>> scores_in(instance.vertex_number());
    for (auto it_v = solution.vertices().out_begin(); it_v != solution.vertices().out_end(); ++it_v) {
        VertexId v = *it_v;
        scores_out.update_key(v, {(double)vertices[v].score / instance.vertex(v).weight, 0});
    }

    optimizationtools::IndexedSet sets_in_to_update(instance.vertex_number());
    optimizationtools::IndexedSet sets_out_to_update(instance.vertex_number());
    Counter iterations_without_improvment = 0;
    for (output.iterations = 1; parameters.info.check_time(); ++output.iterations, ++iterations_without_improvment) {
        // Check stop criteria.
        if (parameters.iteration_limit != -1
                && output.iterations > parameters.iteration_limit)
            break;
        if (parameters.iteration_without_improvment_limit != -1
                && iterations_without_improvment > parameters.iteration_without_improvment_limit)
            break;
        //std::cout
            //<< "weight " << solution.weight()
            //<< " v " << solution.vertex_number()
            //<< " f " << solution.feasible()
            //<< std::endl;

        // Add vertices.
        VertexPos removed_vertex_number = sqrt(instance.vertex_number() - solution.vertex_number());
        //std::cout << "removed_vertex_number " << removed_vertex_number << std::endl;
        sets_in_to_update.clear();
        for (VertexPos s_tmp = 0; s_tmp < removed_vertex_number && !scores_out.empty(); ++s_tmp) {
            auto p = scores_out.top();
            scores_out.pop();
            VertexId v = p.first;
            Weight p_tmp = solution.penalty();
            solution.add(v);
            //std::cout << "add " << v
                //<< " p.second " << p.second.first
                //<< " score " << vertices[v].score
                //<< " weight " << instance.vertex(v).weight
                //<< " p " << solution.penalty()
                //<< " p_tmp " << p_tmp
                //<< std::endl;
            assert(solution.penalty() == p_tmp + vertices[v].score);
            vertices[v].last_addition = output.iterations;
            sets_in_to_update.add(v);
            // Update scores.
            sets_out_to_update.clear();
            for (const auto& edge: instance.vertex(v).edges) {
                if (solution.contains(edge.v)) {
                    vertices[edge.v].score += solution.penalty(edge.e);
                    sets_in_to_update.add(edge.v);
                } else {
                    vertices[edge.v].score += solution.penalty(edge.e);
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
        for (auto it = solution.edges().begin(2); it != solution.edges().end(2); ++it) {
            VertexId v1 = instance.edge(*it).v1;
            VertexId v2 = instance.edge(*it).v2;
            solution.increment_penalty(*it);
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
            Weight p_tmp = solution.penalty();
            solution.remove(v);
            //std::cout << "remove " << v
                //<< " p.second " << p.second.first
                //<< " score " << vertices[v].score
                //<< " weight " << instance.vertex(v).weight
                //<< " p " << solution.penalty()
                //<< " p_tmp " << p_tmp
                //<< std::endl;
            assert(p.second.first < 0);
            assert(solution.penalty() == p_tmp - vertices[v].score);
            vertices[v].last_removal = output.iterations;
            sets_out_to_update.add(v);
            // Update scores.
            sets_in_to_update.clear();
            for (const auto& edge: instance.vertex(v).edges) {
                if (solution.contains(edge.v)) {
                    vertices[edge.v].score -= solution.penalty(edge.e);
                    sets_in_to_update.add(edge.v);
                } else {
                    vertices[edge.v].score -= solution.penalty(edge.e);
                    sets_out_to_update.add(edge.v);
                }
            }

            // Remove redundant sets.
            for (const auto& edge: instance.vertex(v).edges) {
                if (!solution.contains(edge.v) && vertices[edge.v].score == 0) {
                    Weight p_tmp = solution.penalty();
                    solution.add(edge.v);
                    //std::cout << "> add " << edge.v
                        //<< " score " << vertices[edge.v].score
                        //<< " weight " << instance.vertex(edge.v).weight
                        //<< " p " << solution.penalty()
                        //<< std::endl;
                    assert(solution.penalty() == p_tmp);
                    vertices[edge.v].last_addition = output.iterations;
                    sets_in_to_update.add(edge.v);
                    for (const auto& edge_2: instance.vertex(edge.v).edges) {
                        assert(!solution.contains(edge_2.v));
                        vertices[edge_2.v].score += solution.penalty(edge_2.e);
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

