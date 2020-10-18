#include "stablesolver/algorithms/localsearch.hpp"

#include "stablesolver/algorithms/greedy.hpp"

#include <thread>

using namespace stablesolver;

/******************************* localsearch_1 ********************************/

LocalSearch1Output& LocalSearch1Output::algorithm_end(Info& info)
{
    //PUT(info, "Algorithm", "Iterations", iterations);
    Output::algorithm_end(info);
    //VER(info, "Iterations: " << iterations << std::endl);
    return *this;
}

struct LocalSearch1Component
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

struct LocalSearch1Vertex
{
    Counter timestamp = -1;
    Counter last_addition = -1;
    Counter last_removal = -1;
    Counter iterations = 0;
};

void localsearch_1_worker(
        const Instance& instance,
        Seed seed,
        LocalSearch1OptionalParameters parameters,
        LocalSearch1Output& output,
        Counter thread_id)
{
    std::mt19937_64 generator(seed);
    parameters.info.output->mutex_sol.lock();
    Solution solution = output.solution;
    parameters.info.output->mutex_sol.unlock();

    // Initialize local search structures.
    std::vector<LocalSearch1Vertex> vertices(instance.vertex_number());
    for (VertexId v: solution.vertices())
        vertices[v].last_addition = 0;
    std::vector<LocalSearch1Component> components(instance.component_number());
    for (ComponentId c = 0; c < instance.component_number(); ++c)
        components[c].iteration_max = ((c == 0)? 0: components[c - 1].iteration_max)
            + instance.component(c).edges.size();

    ComponentId c = 0;
    for (Counter iterations = 1; parameters.info.check_time(); ++iterations) {
        //std::cout << "it " << iterations << std::endl;

        // Compute component
        if (iterations % (components.back().iteration_max + 1) >= components[c].iteration_max) {
            c = (c + 1) % instance.component_number();
            //std::cout << "c " << c << " " << components[c].iteration_max
                //<< " e " << instance.component(c).edges.size()
                //<< " s " << instance.component(c).vertices.size()
                //<< std::endl;
        }
        LocalSearch1Component& component = components[c];

        while (solution.feasible(c)) {
            // New best solution
            if (output.solution.weight(c) < solution.weight(c)) {
                // Update best solution
                std::stringstream ss;
                ss << "thread " << thread_id
                    << ", it " << iterations
                    << ", comp " << c
                    << " (" << component.iterations_without_improvment << ")";
                output.update_solution(solution, c, ss, parameters.info);
            }
            // Update statistics
            if (component.iterations_without_improvment > 0)
                component.iterations_without_improvment = 0;

            // Find the best shift move.
            VertexId v_best = -1;
            Weight  p_best = -1;
            // For each set s of the current solution which belongs to the
            // currently considered component and is not mandatory.
            for (auto it_v = solution.vertices().out_begin(); it_v != solution.vertices().out_end(); ++it_v) {
                VertexId v = *it_v;
                if (instance.vertex(v).component != c)
                    continue;
                solution.add(v);
                // Update best move.
                if (v_best == -1 // First move considered.
                        || p_best > solution.penalty() // Strictly better.
                        // Equivalent, but s has not been considered for a
                        // longer time.
                        || (p_best == solution.penalty()
                            && vertices[v_best].timestamp > vertices[v].timestamp)) {
                    v_best = v;
                    p_best = solution.penalty();
                }
                solution.remove(v);
            }
            // Apply best move
            solution.add(v_best);
            // Update sets
            vertices[v_best].timestamp = iterations;
            vertices[v_best].last_addition = component.iterations;
            // Update tabu
            component.v_last_added = v_best;
            //std::cout << "it " << iterations
                //<< " v_best " << v_best
                //<< " p " << solution.penalty()
                //<< " v " << solution.vertex_number()
                //<< std::endl;
        }

        // Draw randomly an uncovered edge e.
        std::uniform_int_distribution<EdgeId> d_e(
                0, solution.edges().element_number(2) - 1);
        EdgeId e = *(solution.edges().begin(2) + d_e(generator));
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
        for (VertexId v1: {instance.edge(e).v1, instance.edge(e).v2}) {
            if (v1 == component.v_last_added)
                continue;
            solution.remove(v1);
            if (p_best == -1 || solution.penalty() <= p_best) {
                // For each neighbor s2 of s1 which is neither part of the
                // solution, nor the last set added, nor mandatory.
                for (const auto& edge: instance.vertex(v1).edges) {
                    if (edge.v == component.v_last_removed
                            || solution.contains(edge.v))
                        continue;
                    solution.add(edge.v);
                    // If the new solution is better, we update the best move.
                    if (v1_best == -1 // First move considered.
                            || p_best > solution.penalty() // Strictly better.
                            // Equivalent, but s1 and s2 have not been
                            // considered for a longer time.
                            || (p_best == solution.penalty()
                                && vertices[v1_best].timestamp + vertices[v2_best].timestamp
                                > vertices[v1].timestamp + vertices[edge.v].timestamp)) {
                        v1_best = v1;
                        v2_best = edge.v;
                        p_best = solution.penalty();
                    }
                    solution.remove(edge.v);
                }
            }
            solution.add(v1);
        }
        if (v1_best != -1) {
            // Apply move
            solution.remove(v1_best);
            solution.add(v2_best);
            // Update sets
            vertices[v1_best].timestamp = iterations;
            vertices[v2_best].timestamp = iterations;
            vertices[v1_best].last_removal  = component.iterations;
            vertices[v2_best].last_addition = component.iterations;
            vertices[v1_best].iterations += (component.iterations - vertices[v1_best].last_addition);
        }
        // Update tabu
        component.v_last_removed = v1_best;
        component.v_last_added   = v2_best;
        //std::cout << "it " << iterations
            //<< " v1_best " << v1_best
            //<< " v2_best " << v2_best
            //<< " p " << solution.penalty()
            //<< " v " << solution.vertex_number()
            //<< std::endl;

        // Update penalties: we increment the penalty of each uncovered edge.
        // "reduce" becomes true if we divide by 2 all penalties to avoid
        // integer overflow (this very rarely occur in practice).
        bool reduce = false;
        for (auto it = solution.edges().begin(2); it != solution.edges().end(2); ++it) {
            solution.increment_penalty(*it);
            if (solution.penalty(*it) > std::numeric_limits<Weight>::max() / instance.edge_number())
                reduce = true;
        }
        if (reduce) {
            //std::cout << "reduce" << std::endl;
            for (EdgeId e = 0; e < instance.edge_number(); ++e)
                solution.set_penalty(e, (solution.penalty(e) - 1) / 2 + 1);
        }

        // Update component.iterations and component.iterations_without_improvment.
        component.iterations++;
        component.iterations_without_improvment++;
    }
}

LocalSearch1Output stablesolver::localsearch_1(
        Instance& instance,
        std::mt19937_64& generator,
        LocalSearch1OptionalParameters parameters)
{
    VER(parameters.info, "*** localsearch_1 ***" << std::endl);

    // Compute initial greedy solution.
    LocalSearch1Output output(instance, parameters.info);
    Solution solution = greedy_gwmin(instance).solution;
    std::stringstream ss;
    ss << "initial solution";
    output.update_solution(solution, ss, parameters.info);

    auto seeds = optimizationtools::bob_floyd(parameters.thread_number, std::numeric_limits<Seed>::max(), generator);
    std::vector<std::thread> threads;
    for (Counter thread_id = 0; thread_id < parameters.thread_number; ++thread_id)
        threads.push_back(std::thread(localsearch_1_worker, std::ref(instance), seeds[thread_id], parameters, std::ref(output), thread_id));

    for (Counter thread_id = 0; thread_id < parameters.thread_number; ++thread_id)
        threads[thread_id].join();

    return output.algorithm_end(parameters.info);
}

/******************************* localsearch_2 ********************************/

LocalSearch2Output& LocalSearch2Output::algorithm_end(Info& info)
{
    //PUT(info, "Algorithm", "Iterations", iterations);
    Output::algorithm_end(info);
    //VER(info, "Iterations: " << iterations << std::endl);
    return *this;
}

struct LocalSearch2Vertex
{
    Counter timestamp = -1;
    Counter last_addition = -1;
    Counter last_removal = -1;
    Counter iterations = 0;
    Weight  score = 0;
};

void localsearch_2_worker(
        const Instance& instance,
        Seed seed,
        LocalSearch2OptionalParameters parameters,
        LocalSearch2Output& output,
        Counter thread_id)
{
    std::mt19937_64 generator(seed);
    parameters.info.output->mutex_sol.lock();
    Solution solution = output.solution;
    parameters.info.output->mutex_sol.unlock();

    // Initialize local search structures.
    std::vector<LocalSearch2Vertex> vertices(instance.vertex_number());
    for (EdgeId e = 0; e < instance.edge_number(); ++e) {
        if (solution.covers(e) == 1) {
            if (!solution.contains(instance.edge(e).v1))
                vertices[instance.edge(e).v1].score += solution.penalty(e);
            if (!solution.contains(instance.edge(e).v2))
                vertices[instance.edge(e).v2].score += solution.penalty(e);
        }
    }
    VertexId v_last_removed = -1;
    VertexId v_last_added = -1;

    Counter iterations_without_improvment = 0;
    for (Counter iterations = 1; parameters.info.check_time(); ++iterations, iterations_without_improvment++) {
        //std::cout << "it " << iterations << std::endl;

        while (solution.feasible()) {

            // Update best solution
            if (output.solution.weight() < solution.weight()) {
                std::stringstream ss;
                ss << "thread " << thread_id
                    << ", it " << iterations
                    << " (" << iterations_without_improvment << ")";
                output.update_solution(solution, ss, parameters.info);
            }

            // Update statistics
            iterations_without_improvment = 0;

            // Find the best shift move.
            VertexId   v_best = -1;
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
            Weight tmp = solution.penalty();
            solution.add(v_best);
            //std::cout << "it " << iterations
                //<< " v_best " << v_best
                //<< " score " << score_best
                //<< " p " << solution.penalty()
                //<< " v " << solution.vertex_number()
                //<< " c " << solution.edges().edge_number(2)
                //<< std::endl;
            assert(solution.penalty() == tmp + score_best);
            // Update scores.
            for (const auto& edge: instance.vertex(v_best).edges)
                if (solution.covers(edge.e) >= 1)
                    vertices[edge.v].score += solution.penalty(edge.e);
            // Update vertices
            vertices[v_best].timestamp = iterations;
            vertices[v_best].last_addition = iterations;
            // Update tabu
            v_last_removed = -1;
            v_last_added   = -1;
        }

        // Find the cheapest vertex to add.
        VertexId   v1_best = -1;
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
        Weight tmp1 = solution.penalty();
        solution.add(v1_best);
        //std::cout << "it " << iterations
            //<< " v1_best " << v1_best
            //<< " score " << score1_best
            //<< " p_prec " << tmp1
            //<< " p " << solution.penalty()
            //<< " v " << solution.vertex_number()
            //<< " c " << solution.edges().edge_number(2)
            //<< std::endl;
        assert(solution.penalty() == tmp1 + score1_best);
        // Update scores.
        for (const auto& edge: instance.vertex(v1_best).edges)
            if (solution.covers(edge.e) >= 1)
                vertices[edge.v].score += solution.penalty(edge.e);
        // Update sets
        vertices[v1_best].timestamp = iterations;
        vertices[v1_best].last_addition = iterations;
        // Update tabu
        v_last_added = v1_best;

        // Draw randomly an uncovered edge e.
        std::uniform_int_distribution<EdgeId> d_e(
                0, solution.edges().element_number(2) - 1);
        EdgeId e = *(solution.edges().begin(2) + d_e(generator));
        //std::cout << "it " << iterations
            //<< " e " << e
            //<< " covers " << (int)solution.covers(e)
            //<< std::endl;

        // Find the best vertex to remove.
        VertexId   v2_best = -1;
        Weight score2_best = -1;
        for (VertexId v: {instance.edge(e).v1, instance.edge(e).v2}) {
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
        Weight tmp2 = solution.penalty();
        solution.remove(v2_best);
        //std::cout << "it " << iterations
            //<< " v2_best " << v2_best
            //<< " score " << score2_best
            //<< " p " << solution.penalty()
            //<< " v " << solution.vertex_number()
            //<< " c " << solution.edges().edge_number(2)
            //<< std::endl;
        assert(solution.penalty() == tmp2 - score2_best);
        // Update scores.
        for (const auto& edge: instance.vertex(v2_best).edges)
            if (solution.covers(edge.e) <= 1)
                vertices[edge.v].score -= solution.penalty(edge.e);
        // Update sets
        vertices[v2_best].timestamp = iterations;
        vertices[v2_best].last_removal  = iterations;
        vertices[v2_best].iterations += (iterations - vertices[v2_best].last_addition);
        // Update tabu
        v_last_removed = v2_best;

        // Update penalties: we increment the penalty of each edge with both
        // ends in the solution.
        for (auto it = solution.edges().begin(2); it != solution.edges().end(2); ++it) {
            solution.increment_penalty(*it);
            vertices[instance.edge(*it).v1].score++;
            vertices[instance.edge(*it).v2].score++;
        }
    }
}

LocalSearch2Output stablesolver::localsearch_2(
        const Instance& instance,
        std::mt19937_64& generator,
        LocalSearch2OptionalParameters parameters)
{
    VER(parameters.info, "*** localsearch_2 ***" << std::endl);

    // Compute initial greedy solution.
    LocalSearch2Output output(instance, parameters.info);
    Solution solution = greedy_gwmin(instance).solution;
    std::stringstream ss;
    ss << "initial solution";
    output.update_solution(solution, ss, parameters.info);

    auto seeds = optimizationtools::bob_floyd(parameters.thread_number, std::numeric_limits<Seed>::max(), generator);
    std::vector<std::thread> threads;
    for (Counter thread_id = 0; thread_id < parameters.thread_number; ++thread_id)
        threads.push_back(std::thread(localsearch_2_worker, std::ref(instance), seeds[thread_id], parameters, std::ref(output), thread_id));

    for (Counter thread_id = 0; thread_id < parameters.thread_number; ++thread_id)
        threads[thread_id].join();

    return output.algorithm_end(parameters.info);
}

