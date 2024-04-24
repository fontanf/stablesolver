#include "stablesolver/stable/algorithms/local_search_row_weighting.hpp"

#include "stablesolver/stable/algorithm_formatter.hpp"
#include "stablesolver/stable/algorithms/greedy.hpp"

using namespace stablesolver::stable;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct LocalSearchRowWeighting1Component
{
    /** Last vertex added to the current solution. */
    VertexId vertex_id_last_added = -1;

    /** Last vertex removed from the current solution. */
    VertexId vertex_id_last_removed = -1;

    /** Number of iterations. */
    Counter iterations = 0;

    /** Number of iterations without improvment. */
    Counter iterations_without_improvment = 0;

    /** When to start optimizing next component. */
    Counter iteration_max;
};

struct LocalSearchRowWeighting1Vertex
{
    Counter timestamp = -1;

    Counter last_addition = -1;

    Counter last_removal = -1;

    Counter iterations = 0;
};

const LocalSearchRowWeighting1Output stablesolver::stable::local_search_row_weighting_1(
        const Instance& instance,
        std::mt19937_64& generator,
        const LocalSearchRowWeighting1Parameters& parameters)
{
    LocalSearchRowWeighting1Output output(instance);
    AlgorithmFormatter algorithm_formatter(parameters, output);
    algorithm_formatter.start("Row weighting local search 1");

    // Reduction.
    if (parameters.reduction_parameters.reduce) {
        return solve_reduced_instance(
                [&generator](
                    const Instance& instance,
                    const Parameters& parameters)
                {
                    return local_search_row_weighting_1(
                            instance,
                            generator,
                            static_cast<const LocalSearchRowWeighting1Parameters&>(parameters));
                },
                instance,
                parameters,
                algorithm_formatter,
                output);
    }

    algorithm_formatter.print_header();

    // Compute initial greedy solution.
    GreedyParameters greedy_parameters;
    greedy_parameters.verbosity_level = 0;
    greedy_parameters.timer = parameters.timer;
    greedy_parameters.reduction_parameters.reduce = false;
    Solution solution = greedy_gwmin(instance, greedy_parameters).solution;
    algorithm_formatter.update_solution(solution, "initial solution");

    Solution solution_best(solution);

    if (instance.number_of_vertices() == 0) {
        algorithm_formatter.end();
        return output;
    }

    // Initialize local search structures.
    std::vector<LocalSearchRowWeighting1Vertex> vertices(instance.number_of_vertices());
    for (VertexId vertex_id: solution.vertices())
        vertices[vertex_id].last_addition = 0;
    std::vector<LocalSearchRowWeighting1Component> components(instance.number_of_components());
    for (ComponentId component_id = 0;
            component_id < instance.number_of_components();
            ++component_id) {
        components[component_id].iteration_max
            = ((component_id == 0)? 0: components[component_id - 1].iteration_max)
            + instance.component(component_id).edges.size();
    }
    std::vector<Penalty> solution_penalties(instance.number_of_edges(), 1);

    ComponentId component_id = 0;
    for (output.number_of_iterations = 0;
            !parameters.timer.needs_to_end();
            ++output.number_of_iterations) {
        //std::cout << "it " << iterations << std::endl;
        // Check stop criteria.
        if (parameters.maximum_number_of_iterations != -1
                && output.number_of_iterations >= parameters.maximum_number_of_iterations)
            break;

        // Update best solution
        if (output.number_of_iterations % 100000 == 0) {
            if (optimizationtools::is_solution_strictly_better(
                        objective_direction(),
                        output.solution.feasible(),
                        output.solution.objective_value(),
                        solution_best.feasible(),
                        solution_best.objective_value())) {
                std::stringstream ss;
                ss << "iteration " << output.number_of_iterations;
                algorithm_formatter.update_solution(solution_best, ss.str());
            }
        }

        // Compute component
        if (output.number_of_iterations % (components.back().iteration_max + 1)
                >= components[component_id].iteration_max) {
            component_id = (component_id + 1) % instance.number_of_components();
            while (instance.component(component_id).vertices.size() == 1)
                component_id = (component_id + 1) % instance.number_of_components();
            //std::cout << "c " << c << " " << components[c].iteration_max
                //<< " e " << instance.component(component_id.edges.size()
                //<< " s " << instance.component(component_id.vertices.size()
                //<< std::endl;
        }
        LocalSearchRowWeighting1Component& component = components[component_id];

        while (solution.feasible(component_id)) {
            // New best solution
            if (solution_best.weight(component_id) < solution.weight(component_id)) {
                // Update solution_best.
                for (VertexId vertex_id: instance.component(component_id).vertices) {
                    if (solution_best.contains(vertex_id)
                            && !solution.contains(vertex_id)) {
                        solution_best.remove(vertex_id);
                    } else if (!solution_best.contains(vertex_id)
                            && solution.contains(vertex_id)) {
                        solution_best.add(vertex_id);
                    }
                }
            }
            // Update statistics
            if (component.iterations_without_improvment > 0)
                component.iterations_without_improvment = 0;

            // Find the best shift move.
            VertexId vertex_id_best = -1;
            Weight p_best = -1;
            // For each set s of the current solution which belongs to the
            // currently considered component and is not mandatory.
            for (auto it_v = solution.vertices().out_begin();
                    it_v != solution.vertices().out_end();
                    ++it_v) {
                VertexId vertex_id = *it_v;
                if (instance.vertex(vertex_id).component != component_id)
                    continue;
                Penalty p = 0;
                for (const auto& edge: instance.vertex(vertex_id).edges)
                    if (solution.covers(edge.edge_id) == 1)
                        p += solution_penalties[edge.edge_id];
                // Update best move.
                if (vertex_id_best == -1 // First move considered.
                        || p_best > p // Strictly better.
                        // Equivalent, but s has not been considered for a
                        // longer time.
                        || (p_best == p
                            && vertices[vertex_id_best].timestamp
                            > vertices[vertex_id].timestamp)) {
                    vertex_id_best = vertex_id;
                    p_best = p;
                }
            }
            if (vertex_id_best == -1)
                throw std::runtime_error("vertex_id_best == -1.");
            // Apply best move
            solution.add(vertex_id_best);
            // Update sets
            vertices[vertex_id_best].timestamp = output.number_of_iterations;
            vertices[vertex_id_best].last_addition = component.iterations;
            // Update tabu
            component.vertex_id_last_added = vertex_id_best;
            //std::cout << "it " << iterations
                //<< " vertex_id_best " << vertex_id_best
                //<< " p " << solution.penalty()
                //<< " v " << solution.number_of_vertices()
                //<< std::endl;
        }

        // Draw randomly an uncovered edge e.
        std::uniform_int_distribution<EdgeId> d_e(0, solution.number_of_conflicts() - 1);
        EdgeId edge_id_cur = *std::next(solution.conflicts().begin(), d_e(generator));
        //std::cout << "it " << iterations
            //<< " e " << e
            //<< " covers " << (int)solution.covers(e)
            //<< " vertex_id_1 " << instance.edge(e).vertex_id_1 << " " << (int)solution.contains(instance.edge(e).vertex_id_1)
            //<< " vertex_id_2 " << instance.edge(e).vertex_id_2 << " " << (int)solution.contains(instance.edge(e).vertex_id_2)
            //<< std::endl;

        // Find the best swap move.
        VertexId vertex_id_1_best = -1;
        VertexId vertex_id_2_best = -1;
        Weight    p_best = -1;
        // For each set s1 covering edge e which is not part of the solution
        // and which is not the last set removed.
        for (VertexId vertex_id_1: {instance.edge(edge_id_cur).vertex_id_1, instance.edge(edge_id_cur).vertex_id_2}) {
            if (vertex_id_1 == component.vertex_id_last_added)
                continue;
            Penalty p0 = 0;
            for (const auto& edge: instance.vertex(vertex_id_1).edges)
                if (solution.covers(edge.edge_id) == 2)
                    p0 -= solution_penalties[edge.edge_id];
            solution.remove(vertex_id_1);
            if (p_best == -1 || p0 <= p_best) {
                // For each neighbor s2 of s1 which is neither part of the
                // solution, nor the last set added, nor mandatory.
                for (const auto& edge: instance.vertex(vertex_id_1).edges) {
                    if (edge.vertex_id == component.vertex_id_last_removed
                            || solution.contains(edge.vertex_id))
                        continue;
                    Penalty p = p0;
                    for (const auto& edge_2: instance.vertex(edge.vertex_id).edges)
                        if (solution.covers(edge_2.edge_id) == 1)
                            p += solution_penalties[edge_2.edge_id];
                    // If the new solution is better, we update the best move.
                    if (vertex_id_1_best == -1 // First move considered.
                            || p_best > p // Strictly better.
                            // Equivalent, but s1 and s2 have not been
                            // considered for a longer time.
                            || (p_best == p
                                && vertices[vertex_id_1_best].timestamp
                                + vertices[vertex_id_2_best].timestamp
                                > vertices[vertex_id_1].timestamp
                                + vertices[edge.vertex_id].timestamp)) {
                        vertex_id_1_best = vertex_id_1;
                        vertex_id_2_best = edge.vertex_id;
                        p_best = p;
                    }
                }
            }
            solution.add(vertex_id_1);
        }
        if (vertex_id_1_best != -1) {
            // Apply move
            solution.remove(vertex_id_1_best);
            solution.add(vertex_id_2_best);
            // Update sets
            vertices[vertex_id_1_best].timestamp = output.number_of_iterations;
            vertices[vertex_id_2_best].timestamp = output.number_of_iterations;
            vertices[vertex_id_1_best].last_removal  = component.iterations;
            vertices[vertex_id_2_best].last_addition = component.iterations;
            vertices[vertex_id_1_best].iterations += (component.iterations - vertices[vertex_id_1_best].last_addition);
            // Update penalties.
            bool reduce = false;
            for (const auto& edge: instance.vertex(vertex_id_2_best).edges) {
                if (solution.covers(edge.edge_id) == 2) {
                    solution_penalties[edge.edge_id]++;
                    if (solution_penalties[edge.edge_id]
                            > std::numeric_limits<Penalty>::max() / 2)
                        reduce = true;
                }
            }
            if (reduce) {
                //std::cout << "reduce" << std::endl;
                for (EdgeId edge_id = 0;
                        edge_id < instance.number_of_edges();
                        ++edge_id) {
                    solution_penalties[edge_id] = (solution_penalties[edge_id] - 1) / 2 + 1;
                }
            }
        }
        // Update tabu
        component.vertex_id_last_removed = vertex_id_1_best;
        component.vertex_id_last_added   = vertex_id_2_best;
        //std::cout << "it " << iterations
            //<< " vertex_id_1_best " << vertex_id_1_best
            //<< " vertex_id_2_best " << vertex_id_2_best
            //<< " p " << solution.penalty()
            //<< " v " << solution.number_of_vertices()
            //<< std::endl;

        // Update component.iterations and component.iterations_without_improvment.
        component.iterations++;
        component.iterations_without_improvment++;
    }

    algorithm_formatter.end();
    return output;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////// local_search_row_weighting_2 //////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct LocalSearchRowWeighting2Vertex
{
    Counter timestamp = -1;

    Counter last_addition = -1;

    Counter last_removal = -1;

    Counter iterations = 0;

    Weight  score = 0;
};

const LocalSearchRowWeighting2Output stablesolver::stable::local_search_row_weighting_2(
        const Instance& instance,
        std::mt19937_64& generator,
        const LocalSearchRowWeighting2Parameters& parameters)
{
    LocalSearchRowWeighting2Output output(instance);
    AlgorithmFormatter algorithm_formatter(parameters, output);
    algorithm_formatter.start("Row weighting local search 2");

    // Reduction.
    if (parameters.reduction_parameters.reduce) {
        return solve_reduced_instance(
                [&generator](
                    const Instance& instance,
                    const Parameters& parameters)
                {
                    return local_search_row_weighting_2(
                            instance,
                            generator,
                            static_cast<const LocalSearchRowWeighting2Parameters&>(parameters));
                },
                instance,
                parameters,
                algorithm_formatter,
                output);
    }

    algorithm_formatter.print_header();

    // Compute initial greedy solution.
    GreedyParameters greedy_parameters;
    greedy_parameters.verbosity_level = 0;
    greedy_parameters.timer = parameters.timer;
    greedy_parameters.reduction_parameters.reduce = false;
    Solution solution = greedy_gwmin(instance, greedy_parameters).solution;
    algorithm_formatter.update_solution(solution, "initial solution");

    if (instance.number_of_vertices() == 0) {
        algorithm_formatter.end();
        return output;
    }

    // Initialize local search structures.
    std::vector<LocalSearchRowWeighting2Vertex> vertices(instance.number_of_vertices());
    std::vector<Penalty> solution_penalties(instance.number_of_edges(), 1);
    for (EdgeId edge_id = 0; edge_id < instance.number_of_edges(); ++edge_id) {
        if (solution.covers(edge_id) == 1) {
            if (!solution.contains(instance.edge(edge_id).vertex_id_1))
                vertices[instance.edge(edge_id).vertex_id_1].score
                    += solution_penalties[edge_id];
            if (!solution.contains(instance.edge(edge_id).vertex_id_2))
                vertices[instance.edge(edge_id).vertex_id_2].score
                    += solution_penalties[edge_id];
        }
    }
    VertexId vertex_id_last_removed = -1;
    VertexId vertex_id_last_added = -1;

    Counter iterations_without_improvment = 0;
    for (output.number_of_iterations = 0;
            !parameters.timer.needs_to_end();
            ++output.number_of_iterations,
            ++iterations_without_improvment) {
        // Check stop criteria.
        if (parameters.maximum_number_of_iterations != -1
                && output.number_of_iterations >= parameters.maximum_number_of_iterations)
            break;
        if (parameters.maximum_number_of_iterations_without_improvement != -1
                && iterations_without_improvment >= parameters.maximum_number_of_iterations_without_improvement)
            break;
        //std::cout << "it " << iterations << std::endl;

        while (solution.feasible()) {

            // Update best solution
            if (optimizationtools::is_solution_strictly_better(
                        objective_direction(),
                        output.solution.feasible(),
                        output.solution.objective_value(),
                        solution.feasible(),
                        solution.objective_value())) {
                std::stringstream ss;
                ss << "iteration " << output.number_of_iterations;
                algorithm_formatter.update_solution(solution, ss.str());
            }

            // Update statistics
            iterations_without_improvment = 0;

            // Find the best shift move.
            VertexId vertex_id_best = -1;
            Weight score_best = -1;
            for (auto it_v = solution.vertices().out_begin();
                    it_v != solution.vertices().out_end();
                    ++it_v) {
                if (vertex_id_best == -1
                        || score_best > vertices[*it_v].score
                        || (score_best == vertices[*it_v].score
                            && vertices[vertex_id_best].timestamp > vertices[*it_v].timestamp)) {
                    vertex_id_best = *it_v;
                    score_best = vertices[*it_v].score;
                }
            }
            // Apply best move
            solution.add(vertex_id_best);
            //std::cout << "it " << iterations
                //<< " vertex_id_best " << vertex_id_best
                //<< " score " << score_best
                //<< " p " << solution.penalty()
                //<< " v " << solution.number_of_vertices()
                //<< " c " << solution.edges().number_of_edges(2)
                //<< std::endl;
            // Update scores.
            for (const auto& edge: instance.vertex(vertex_id_best).edges)
                if (solution.covers(edge.edge_id) >= 1)
                    vertices[edge.vertex_id].score += solution_penalties[edge.edge_id];
            // Update vertices
            vertices[vertex_id_best].timestamp = output.number_of_iterations;
            vertices[vertex_id_best].last_addition = output.number_of_iterations;
            // Update tabu
            vertex_id_last_removed = -1;
            vertex_id_last_added   = -1;
            // Update penalties.
            for (const auto& edge: instance.vertex(vertex_id_best).edges) {
                if (solution.covers(edge.edge_id) >= 2) {
                    solution_penalties[edge.edge_id]++;
                    vertices[instance.edge(edge.edge_id).vertex_id_1].score++;
                    vertices[instance.edge(edge.edge_id).vertex_id_2].score++;
                }
            }
        }

        // Find the cheapest vertex to add.
        VertexId vertex_id_1_best = -1;
        Weight score1_best = -1;
        for (auto it_v = solution.vertices().out_begin();
                it_v != solution.vertices().out_end();
                ++it_v) {
            VertexId vertex_id = *it_v;
            if (vertex_id == vertex_id_last_removed)
                continue;
            if (vertex_id_1_best == -1
                    || score1_best > vertices[vertex_id].score
                    || (score1_best == vertices[vertex_id].score
                        && vertices[vertex_id_1_best].timestamp
                        > vertices[vertex_id].timestamp)) {
                vertex_id_1_best = vertex_id;
                score1_best = vertices[vertex_id].score;
            }
        }
        // Apply move
        solution.add(vertex_id_1_best);
        //std::cout << "it " << iterations
            //<< " vertex_id_1_best " << vertex_id_1_best
            //<< " score " << score1_best
            //<< " p_prec " << tmp1
            //<< " p " << solution.penalty()
            //<< " v " << solution.number_of_vertices()
            //<< " c " << solution.edges().number_of_edges(2)
            //<< std::endl;
        // Update scores.
        for (const auto& edge: instance.vertex(vertex_id_1_best).edges)
            if (solution.covers(edge.edge_id) >= 1)
                vertices[edge.vertex_id].score += solution_penalties[edge.edge_id];
        // Update sets
        vertices[vertex_id_1_best].timestamp = output.number_of_iterations;
        vertices[vertex_id_1_best].last_addition = output.number_of_iterations;
        // Update tabu
        vertex_id_last_added = vertex_id_1_best;
        // Update penalties.
        for (const auto& edge: instance.vertex(vertex_id_1_best).edges) {
            if (solution.covers(edge.edge_id) >= 2) {
                solution_penalties[edge.edge_id]++;
                vertices[instance.edge(edge.edge_id).vertex_id_1].score++;
                vertices[instance.edge(edge.edge_id).vertex_id_2].score++;
            }
        }

        // Draw randomly an uncovered edge e.
        std::uniform_int_distribution<EdgeId> d_e(0, solution.number_of_conflicts() - 1);
        EdgeId edge_id_cur = *std::next(solution.conflicts().begin(), d_e(generator));
        //std::cout << "it " << iterations
            //<< " e " << e
            //<< " covers " << (int)solution.covers(e)
            //<< std::endl;

        // Find the best vertex to remove.
        VertexId vertex_id_2_best = -1;
        Weight score2_best = -1;
        for (VertexId vertex_id: {instance.edge(edge_id_cur).vertex_id_1, instance.edge(edge_id_cur).vertex_id_2}) {
            if (vertex_id == vertex_id_last_added)
                continue;
            if (vertex_id_2_best == -1
                    || score2_best < vertices[vertex_id].score
                    || (score2_best == vertices[vertex_id].score
                        && vertices[vertex_id_2_best].timestamp
                        > vertices[vertex_id].timestamp)) {
                vertex_id_2_best = vertex_id;
                score2_best = vertices[vertex_id].score;
            }
        }
        if (vertex_id_2_best == -1)
            vertex_id_2_best = vertex_id_1_best;
        // Apply move
        solution.remove(vertex_id_2_best);
        //std::cout << "it " << iterations
            //<< " vertex_id_2_best " << vertex_id_2_best
            //<< " score " << score2_best
            //<< " p " << solution.penalty()
            //<< " v " << solution.number_of_vertices()
            //<< " c " << solution.edges().number_of_edges(2)
            //<< std::endl;
        // Update scores.
        for (const auto& edge: instance.vertex(vertex_id_2_best).edges)
            if (solution.covers(edge.edge_id) <= 1)
                vertices[edge.vertex_id].score -= solution_penalties[edge.edge_id];
        // Update sets
        vertices[vertex_id_2_best].timestamp = output.number_of_iterations;
        vertices[vertex_id_2_best].last_removal  = output.number_of_iterations;
        vertices[vertex_id_2_best].iterations += (output.number_of_iterations - vertices[vertex_id_2_best].last_addition);
        // Update tabu
        vertex_id_last_removed = vertex_id_2_best;
    }

    algorithm_formatter.end();
    return output;
}
