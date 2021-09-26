#include "stablesolver/algorithms/localsearch_rowweighting.hpp"

#include "stablesolver/algorithms/greedy.hpp"

#include <thread>

using namespace stablesolver;

/******************************* localsearch_rowweighting_1 ********************************/

LocalSearchRowWeighting1Output& LocalSearchRowWeighting1Output::algorithm_end(
        optimizationtools::Info& info)
{
    //PUT(info, "Algorithm", "Iterations", iterations);
    Output::algorithm_end(info);
    //VER(info, "Iterations: " << iterations << std::endl);
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

void localsearch_rowweighting_1_worker(
        const Instance& instance,
        Seed seed,
        LocalSearchRowWeighting1OptionalParameters parameters,
        LocalSearchRowWeighting1Output& output,
        Counter thread_id)
{
    std::mt19937_64 generator(seed);
    parameters.info.output->mutex_solutions.lock();
    Solution solution = output.solution;
    parameters.info.output->mutex_solutions.unlock();

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
    for (Counter iterations = 1; !parameters.info.needs_to_end(); ++iterations) {
        //std::cout << "it " << iterations << std::endl;

        // Compute component
        if (iterations % (components.back().iteration_max + 1) >= components[c].iteration_max) {
            c = (c + 1) % instance.number_of_components();
            //std::cout << "c " << c << " " << components[c].iteration_max
                //<< " e " << instance.component(c).edges.size()
                //<< " s " << instance.component(c).vertices.size()
                //<< std::endl;
        }
        LocalSearchRowWeighting1Component& component = components[c];

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
                parameters.info.output->mutex_solutions.lock();
                parameters.new_solution_callback(output);
                parameters.info.output->mutex_solutions.unlock();
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
                //<< " v " << solution.number_of_vertices()
                //<< std::endl;
        }

        // Draw randomly an uncovered edge e.
        std::uniform_int_distribution<EdgeId> d_e(
                0, solution.edges().number_of_elements(2) - 1);
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
            //<< " v " << solution.number_of_vertices()
            //<< std::endl;

        // Update penalties: we increment the penalty of each uncovered edge.
        // "reduce" becomes true if we divide by 2 all penalties to avoid
        // integer overflow (this very rarely occur in practice).
        bool reduce = false;
        for (auto it = solution.edges().begin(2); it != solution.edges().end(2); ++it) {
            solution_penalties[*it]++;
            if (solution_penalties[*it] > std::numeric_limits<Weight>::max() / 2)
                reduce = true;
        }
        if (reduce) {
            //std::cout << "reduce" << std::endl;
            for (EdgeId e = 0; e < instance.number_of_edges(); ++e)
                solution_penalties[e] = (solution_penalties[e] - 1) / 2 + 1;
        }

        // Update component.iterations and component.iterations_without_improvment.
        component.iterations++;
        component.iterations_without_improvment++;
    }
}

LocalSearchRowWeighting1Output stablesolver::localsearch_rowweighting_1(
        Instance& instance,
        std::mt19937_64& generator,
        LocalSearchRowWeighting1OptionalParameters parameters)
{
    VER(parameters.info, "*** localsearch_rowweighting_1 ***" << std::endl);

    // Compute initial greedy solution.
    LocalSearchRowWeighting1Output output(instance, parameters.info);
    Solution solution = greedy_gwmin(instance).solution;
    std::stringstream ss;
    ss << "initial solution";
    output.update_solution(solution, ss, parameters.info);
    parameters.info.output->mutex_solutions.lock();
    parameters.new_solution_callback(output);
    parameters.info.output->mutex_solutions.unlock();

    auto seeds = optimizationtools::bob_floyd(parameters.number_of_threads, std::numeric_limits<Seed>::max(), generator);
    std::vector<std::thread> threads;
    for (Counter thread_id = 0; thread_id < parameters.number_of_threads; ++thread_id)
        threads.push_back(std::thread(localsearch_rowweighting_1_worker, std::ref(instance), seeds[thread_id], parameters, std::ref(output), thread_id));

    for (Counter thread_id = 0; thread_id < parameters.number_of_threads; ++thread_id)
        threads[thread_id].join();

    return output.algorithm_end(parameters.info);
}

/******************************* localsearch_rowweighting_2 ********************************/

LocalSearchRowWeighting2Output& LocalSearchRowWeighting2Output::algorithm_end(
        optimizationtools::Info& info)
{
    //PUT(info, "Algorithm", "Iterations", iterations);
    Output::algorithm_end(info);
    //VER(info, "Iterations: " << iterations << std::endl);
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

void localsearch_rowweighting_2_worker(
        const Instance& instance,
        Seed seed,
        LocalSearchRowWeighting2OptionalParameters parameters,
        LocalSearchRowWeighting2Output& output,
        Counter thread_id)
{
    std::mt19937_64 generator(seed);
    parameters.info.output->mutex_solutions.lock();
    Solution solution = output.solution;
    parameters.info.output->mutex_solutions.unlock();

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
    for (Counter iterations = 1; !parameters.info.needs_to_end(); ++iterations, iterations_without_improvment++) {
        //std::cout << "it " << iterations << std::endl;

        while (solution.feasible()) {

            // Update best solution
            if (output.solution.weight() < solution.weight()) {
                std::stringstream ss;
                ss << "thread " << thread_id
                    << ", it " << iterations
                    << " (" << iterations_without_improvment << ")";
                output.update_solution(solution, ss, parameters.info);
                parameters.info.output->mutex_solutions.lock();
                parameters.new_solution_callback(output);
                parameters.info.output->mutex_solutions.unlock();
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
        vertices[v1_best].timestamp = iterations;
        vertices[v1_best].last_addition = iterations;
        // Update tabu
        v_last_added = v1_best;

        // Draw randomly an uncovered edge e.
        std::uniform_int_distribution<EdgeId> d_e(
                0, solution.edges().number_of_elements(2) - 1);
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
        vertices[v2_best].timestamp = iterations;
        vertices[v2_best].last_removal  = iterations;
        vertices[v2_best].iterations += (iterations - vertices[v2_best].last_addition);
        // Update tabu
        v_last_removed = v2_best;

        // Update penalties: we increment the penalty of each edge with both
        // ends in the solution.
        for (auto it = solution.edges().begin(2); it != solution.edges().end(2); ++it) {
            solution_penalties[*it]++;
            vertices[instance.edge(*it).v1].score++;
            vertices[instance.edge(*it).v2].score++;
        }
    }
}

LocalSearchRowWeighting2Output stablesolver::localsearch_rowweighting_2(
        const Instance& instance,
        std::mt19937_64& generator,
        LocalSearchRowWeighting2OptionalParameters parameters)
{
    VER(parameters.info, "*** localsearch_rowweighting_2 ***" << std::endl);

    // Compute initial greedy solution.
    LocalSearchRowWeighting2Output output(instance, parameters.info);
    Solution solution = greedy_gwmin(instance).solution;
    std::stringstream ss;
    ss << "initial solution";
    output.update_solution(solution, ss, parameters.info);
    parameters.info.output->mutex_solutions.lock();
    parameters.new_solution_callback(output);
    parameters.info.output->mutex_solutions.unlock();

    auto seeds = optimizationtools::bob_floyd(parameters.number_of_threads, std::numeric_limits<Seed>::max(), generator);
    std::vector<std::thread> threads;
    for (Counter thread_id = 0; thread_id < parameters.number_of_threads; ++thread_id)
        threads.push_back(std::thread(localsearch_rowweighting_2_worker, std::ref(instance), seeds[thread_id], parameters, std::ref(output), thread_id));

    for (Counter thread_id = 0; thread_id < parameters.number_of_threads; ++thread_id)
        threads[thread_id].join();

    return output.algorithm_end(parameters.info);
}

