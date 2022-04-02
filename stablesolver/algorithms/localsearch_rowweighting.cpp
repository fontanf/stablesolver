#include "stablesolver/algorithms/localsearch_rowweighting.hpp"

#include "stablesolver/algorithms/greedy.hpp"

#include <thread>

using namespace stablesolver;

////////////////////////////////////////////////////////////////////////////////
////////////////////////// localsearch_rowweighting_1 //////////////////////////
////////////////////////////////////////////////////////////////////////////////

LocalSearchRowWeighting1Output& LocalSearchRowWeighting1Output::algorithm_end(
        optimizationtools::Info& info)
{
    PUT(info, "Algorithm", "NumberOfIterations", number_of_iterations);
    Output::algorithm_end(info);
    VER(info, "Number of iterations:  " << number_of_iterations << std::endl);
    return *this;
}

struct LocalSearchRowWeighting1Component
{
    /** Last vertex added to the current solution. */
    VertexId v_last_added = -1;
    /** Last vertex removed from the current solution. */
    VertexId v_last_removed = -1;
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

LocalSearchRowWeighting1Output stablesolver::localsearch_rowweighting_1(
        Instance& instance_original,
        std::mt19937_64& generator,
        LocalSearchRowWeighting1OptionalParameters parameters)
{
    init_display(instance_original, parameters.info);
    VER(parameters.info,
               "Algorithm" << std::endl
            << "---------" << std::endl
            << "Row Weighting Local Search 1" << std::endl
            << std::endl);

    // Compute initial greedy solution.
    LocalSearchRowWeighting1Output output(instance_original, parameters.info);
    const Instance& instance = (instance_original.reduced_instance() == nullptr)?  instance_original: *instance_original.reduced_instance();
    Solution solution = greedy_gwmin(instance).solution;
    std::stringstream ss;
    ss << "initial solution";
    output.update_solution(solution, ss, parameters.info);
    parameters.new_solution_callback(output);
    Solution solution_best(solution);

    if (instance.number_of_vertices() == 0)
        return output.algorithm_end(parameters.info);

    // Initialize local search structures.
    std::vector<LocalSearchRowWeighting1Vertex> vertices(instance.number_of_vertices());
    for (VertexId v: solution.vertices())
        vertices[v].last_addition = 0;
    std::vector<LocalSearchRowWeighting1Component> components(instance.number_of_components());
    for (ComponentId c = 0; c < instance.number_of_components(); ++c)
        components[c].iteration_max = ((c == 0)? 0: components[c - 1].iteration_max)
            + instance.component(c).edges.size();
    std::vector<Penalty> solution_penalties(instance.number_of_edges(), 1);

    ComponentId c = 0;
    for (output.number_of_iterations = 0; !parameters.info.needs_to_end(); ++output.number_of_iterations) {
        //std::cout << "it " << iterations << std::endl;

        // Update best solution
        if (output.number_of_iterations % 100000 == 0
                && solution_best.is_striclty_better_than(output.solution)) {
            std::stringstream ss;
            ss << "iteration " << output.number_of_iterations;
            output.update_solution(solution_best, ss, parameters.info);
            parameters.info.output->mutex_solutions.lock();
            parameters.new_solution_callback(output);
            parameters.info.output->mutex_solutions.unlock();
        }

        // Compute component
        if (output.number_of_iterations % (components.back().iteration_max + 1) >= components[c].iteration_max) {
            c = (c + 1) % instance.number_of_components();
            while (instance.component(c).vertices.size() == 1)
                c = (c + 1) % instance.number_of_components();
            //std::cout << "c " << c << " " << components[c].iteration_max
                //<< " e " << instance.component(c).edges.size()
                //<< " s " << instance.component(c).vertices.size()
                //<< std::endl;
        }
        LocalSearchRowWeighting1Component& component = components[c];

        while (solution.feasible(c)) {
            // New best solution
            if (solution_best.weight(c) < solution.weight(c)) {
                // Update solution_best.
                for (VertexId v: instance.component(c).vertices) {
                    if (solution_best.contains(v) && !solution.contains(v)) {
                        solution_best.remove(v);
                    } else if (!solution_best.contains(v) && solution.contains(v)) {
                        solution_best.add(v);
                    }
                }
            }
            // Update statistics
            if (component.iterations_without_improvment > 0)
                component.iterations_without_improvment = 0;

            // Find the best shift move.
            VertexId v_best = -1;
            Weight p_best = -1;
            // For each set s of the current solution which belongs to the
            // currently considered component and is not mandatory.
            for (auto it_v = solution.vertices().out_begin(); it_v != solution.vertices().out_end(); ++it_v) {
                VertexId v = *it_v;
                if (instance.vertex(v).component != c)
                    continue;
                Penalty p = 0;
                for (const auto& edge: instance.vertex(v).edges)
                    if (solution.covers(edge.e) == 1)
                        p += solution_penalties[edge.e];
                // Update best move.
                if (v_best == -1 // First move considered.
                        || p_best > p // Strictly better.
                        // Equivalent, but s has not been considered for a
                        // longer time.
                        || (p_best == p
                            && vertices[v_best].timestamp > vertices[v].timestamp)) {
                    v_best = v;
                    p_best = p;
                }
            }
            if (v_best == -1)
                throw std::runtime_error("v_best == -1.");
            // Apply best move
            solution.add(v_best);
            // Update sets
            vertices[v_best].timestamp = output.number_of_iterations;
            vertices[v_best].last_addition = component.iterations;
            // Update tabu
            component.v_last_added = v_best;
            //std::cout << "it " << iterations
                //<< " v_best " << v_best
                //<< " p " << solution.penalty()
                //<< " v " << solution.number_of_vertices()
                //<< std::endl;
        }

        // Draw randomly an uncovered edge e.
        std::uniform_int_distribution<EdgeId> d_e(0, solution.number_of_conflicts() - 1);
        EdgeId e_cur = *std::next(solution.conflicts().begin(), d_e(generator));
        //std::cout << "it " << iterations
            //<< " e " << e
            //<< " covers " << (int)solution.covers(e)
            //<< " v1 " << instance.edge(e).v1 << " " << (int)solution.contains(instance.edge(e).v1)
            //<< " v2 " << instance.edge(e).v2 << " " << (int)solution.contains(instance.edge(e).v2)
            //<< std::endl;

        // Find the best swap move.
        VertexId v1_best = -1;
        VertexId v2_best = -1;
        Weight    p_best = -1;
        // For each set s1 covering edge e which is not part of the solution
        // and which is not the last set removed.
        for (VertexId v1: {instance.edge(e_cur).v1, instance.edge(e_cur).v2}) {
            if (v1 == component.v_last_added)
                continue;
            Penalty p0 = 0;
            for (const auto& edge: instance.vertex(v1).edges)
                if (solution.covers(edge.e) == 2)
                    p0 -= solution_penalties[edge.e];
            solution.remove(v1);
            if (p_best == -1 || p0 <= p_best) {
                // For each neighbor s2 of s1 which is neither part of the
                // solution, nor the last set added, nor mandatory.
                for (const auto& edge: instance.vertex(v1).edges) {
                    if (edge.v == component.v_last_removed
                            || solution.contains(edge.v))
                        continue;
                    Penalty p = p0;
                    for (const auto& edge_2: instance.vertex(edge.v).edges)
                        if (solution.covers(edge_2.e) == 1)
                            p += solution_penalties[edge_2.e];
                    // If the new solution is better, we update the best move.
                    if (v1_best == -1 // First move considered.
                            || p_best > p // Strictly better.
                            // Equivalent, but s1 and s2 have not been
                            // considered for a longer time.
                            || (p_best == p
                                && vertices[v1_best].timestamp + vertices[v2_best].timestamp
                                > vertices[v1].timestamp + vertices[edge.v].timestamp)) {
                        v1_best = v1;
                        v2_best = edge.v;
                        p_best = p;
                    }
                }
            }
            solution.add(v1);
        }
        if (v1_best != -1) {
            // Apply move
            solution.remove(v1_best);
            solution.add(v2_best);
            // Update sets
            vertices[v1_best].timestamp = output.number_of_iterations;
            vertices[v2_best].timestamp = output.number_of_iterations;
            vertices[v1_best].last_removal  = component.iterations;
            vertices[v2_best].last_addition = component.iterations;
            vertices[v1_best].iterations += (component.iterations - vertices[v1_best].last_addition);
            // Update penalties.
            bool reduce = false;
            for (const auto& edge: instance.vertex(v2_best).edges) {
                if (solution.covers(edge.e) == 2) {
                    solution_penalties[edge.e]++;
                    if (solution_penalties[edge.e] > std::numeric_limits<Penalty>::max() / 2)
                        reduce = true;
                }
            }
            if (reduce) {
                //std::cout << "reduce" << std::endl;
                for (EdgeId e = 0; e < instance.number_of_edges(); ++e)
                    solution_penalties[e] = (solution_penalties[e] - 1) / 2 + 1;
            }
        }
        // Update tabu
        component.v_last_removed = v1_best;
        component.v_last_added   = v2_best;
        //std::cout << "it " << iterations
            //<< " v1_best " << v1_best
            //<< " v2_best " << v2_best
            //<< " p " << solution.penalty()
            //<< " v " << solution.number_of_vertices()
            //<< std::endl;

        // Update component.iterations and component.iterations_without_improvment.
        component.iterations++;
        component.iterations_without_improvment++;
    }

    return output.algorithm_end(parameters.info);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////// localsearch_rowweighting_2 //////////////////////////
////////////////////////////////////////////////////////////////////////////////

LocalSearchRowWeighting2Output& LocalSearchRowWeighting2Output::algorithm_end(
        optimizationtools::Info& info)
{
    PUT(info, "Algorithm", "NumberOfIterations", number_of_iterations);
    Output::algorithm_end(info);
    VER(info, "Number of iterations:  " << number_of_iterations << std::endl);
    return *this;
}

struct LocalSearchRowWeighting2Vertex
{
    Counter timestamp = -1;
    Counter last_addition = -1;
    Counter last_removal = -1;
    Counter iterations = 0;
    Weight  score = 0;
};

LocalSearchRowWeighting2Output stablesolver::localsearch_rowweighting_2(
        const Instance& instance_original,
        std::mt19937_64& generator,
        LocalSearchRowWeighting2OptionalParameters parameters)
{
    init_display(instance_original, parameters.info);
    VER(parameters.info,
               "Algorithm" << std::endl
            << "---------" << std::endl
            << "Row Weighting Local Search 1" << std::endl
            << std::endl);

    // Compute initial greedy solution.
    LocalSearchRowWeighting2Output output(instance_original, parameters.info);
    const Instance& instance = (instance_original.reduced_instance() == nullptr)?  instance_original: *instance_original.reduced_instance();
    Solution solution = greedy_gwmin(instance).solution;
    std::stringstream ss;
    ss << "initial solution";
    output.update_solution(solution, ss, parameters.info);
    parameters.info.output->mutex_solutions.lock();
    parameters.new_solution_callback(output);
    parameters.info.output->mutex_solutions.unlock();

    if (instance.number_of_vertices() == 0)
        return output.algorithm_end(parameters.info);

    // Initialize local search structures.
    std::vector<LocalSearchRowWeighting2Vertex> vertices(instance.number_of_vertices());
    std::vector<Penalty> solution_penalties(instance.number_of_edges(), 1);
    for (EdgeId e = 0; e < instance.number_of_edges(); ++e) {
        if (solution.covers(e) == 1) {
            if (!solution.contains(instance.edge(e).v1))
                vertices[instance.edge(e).v1].score += solution_penalties[e];
            if (!solution.contains(instance.edge(e).v2))
                vertices[instance.edge(e).v2].score += solution_penalties[e];
        }
    }
    VertexId v_last_removed = -1;
    VertexId v_last_added = -1;

    Counter iterations_without_improvment = 0;
    for (output.number_of_iterations = 0; !parameters.info.needs_to_end(); ++output.number_of_iterations, iterations_without_improvment++) {
        //std::cout << "it " << iterations << std::endl;

        while (solution.feasible()) {
            // Update best solution
            if (solution.is_striclty_better_than(output.solution)) {
                std::stringstream ss;
                ss << "iteration " << output.number_of_iterations;
                output.update_solution(solution, ss, parameters.info);
                parameters.info.output->mutex_solutions.lock();
                parameters.new_solution_callback(output);
                parameters.info.output->mutex_solutions.unlock();
            }

            // Update statistics
            iterations_without_improvment = 0;

            // Find the best shift move.
            VertexId v_best = -1;
            Weight score_best = -1;
            for (auto it_v = solution.vertices().out_begin(); it_v != solution.vertices().out_end(); ++it_v) {
                if (v_best == -1
                        || score_best > vertices[*it_v].score
                        || (score_best == vertices[*it_v].score
                            && vertices[v_best].timestamp > vertices[*it_v].timestamp)) {
                    v_best = *it_v;
                    score_best = vertices[*it_v].score;
                }
            }
            // Apply best move
            solution.add(v_best);
            //std::cout << "it " << iterations
                //<< " v_best " << v_best
                //<< " score " << score_best
                //<< " p " << solution.penalty()
                //<< " v " << solution.number_of_vertices()
                //<< " c " << solution.edges().number_of_edges(2)
                //<< std::endl;
            // Update scores.
            for (const auto& edge: instance.vertex(v_best).edges)
                if (solution.covers(edge.e) >= 1)
                    vertices[edge.v].score += solution_penalties[edge.e];
            // Update vertices
            vertices[v_best].timestamp = output.number_of_iterations;
            vertices[v_best].last_addition = output.number_of_iterations;
            // Update tabu
            v_last_removed = -1;
            v_last_added   = -1;
            // Update penalties.
            for (const auto& edge: instance.vertex(v_best).edges) {
                if (solution.covers(edge.e) >= 2) {
                    solution_penalties[edge.e]++;
                    vertices[instance.edge(edge.e).v1].score++;
                    vertices[instance.edge(edge.e).v2].score++;
                }
            }
        }

        // Find the cheapest vertex to add.
        VertexId v1_best = -1;
        Weight score1_best = -1;
        for (auto it_v = solution.vertices().out_begin(); it_v != solution.vertices().out_end(); ++it_v) {
            VertexId v = *it_v;
            if (v == v_last_removed)
                continue;
            if (v1_best == -1
                    || score1_best > vertices[v].score
                    || (score1_best == vertices[v].score
                        && vertices[v1_best].timestamp > vertices[v].timestamp)) {
                v1_best = v;
                score1_best = vertices[v].score;
            }
        }
        // Apply move
        solution.add(v1_best);
        //std::cout << "it " << iterations
            //<< " v1_best " << v1_best
            //<< " score " << score1_best
            //<< " p_prec " << tmp1
            //<< " p " << solution.penalty()
            //<< " v " << solution.number_of_vertices()
            //<< " c " << solution.edges().number_of_edges(2)
            //<< std::endl;
        // Update scores.
        for (const auto& edge: instance.vertex(v1_best).edges)
            if (solution.covers(edge.e) >= 1)
                vertices[edge.v].score += solution_penalties[edge.e];
        // Update sets
        vertices[v1_best].timestamp = output.number_of_iterations;
        vertices[v1_best].last_addition = output.number_of_iterations;
        // Update tabu
        v_last_added = v1_best;
        // Update penalties.
        for (const auto& edge: instance.vertex(v1_best).edges) {
            if (solution.covers(edge.e) >= 2) {
                solution_penalties[edge.e]++;
                vertices[instance.edge(edge.e).v1].score++;
                vertices[instance.edge(edge.e).v2].score++;
            }
        }

        // Draw randomly an uncovered edge e.
        std::uniform_int_distribution<EdgeId> d_e(0, solution.number_of_conflicts() - 1);
        EdgeId e_cur = *std::next(solution.conflicts().begin(), d_e(generator));
        //std::cout << "it " << iterations
            //<< " e " << e
            //<< " covers " << (int)solution.covers(e)
            //<< std::endl;

        // Find the best vertex to remove.
        VertexId v2_best = -1;
        Weight score2_best = -1;
        for (VertexId v: {instance.edge(e_cur).v1, instance.edge(e_cur).v2}) {
            if (v == v_last_added)
                continue;
            if (v2_best == -1
                    || score2_best < vertices[v].score
                    || (score2_best == vertices[v].score
                        && vertices[v2_best].timestamp > vertices[v].timestamp)) {
                v2_best = v;
                score2_best = vertices[v].score;
            }
        }
        if (v2_best == -1)
            v2_best = v1_best;
        // Apply move
        solution.remove(v2_best);
        //std::cout << "it " << iterations
            //<< " v2_best " << v2_best
            //<< " score " << score2_best
            //<< " p " << solution.penalty()
            //<< " v " << solution.number_of_vertices()
            //<< " c " << solution.edges().number_of_edges(2)
            //<< std::endl;
        // Update scores.
        for (const auto& edge: instance.vertex(v2_best).edges)
            if (solution.covers(edge.e) <= 1)
                vertices[edge.v].score -= solution_penalties[edge.e];
        // Update sets
        vertices[v2_best].timestamp = output.number_of_iterations;
        vertices[v2_best].last_removal  = output.number_of_iterations;
        vertices[v2_best].iterations += (output.number_of_iterations - vertices[v2_best].last_addition);
        // Update tabu
        v_last_removed = v2_best;
    }

    return output.algorithm_end(parameters.info);
}

