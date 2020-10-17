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

struct LocalSearch1Vertex
{
    Counter timestamp = -1;
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
    Counter iterations_without_improvment = 0;

    for (Counter iterations = 1; parameters.info.check_time(); ++iterations, iterations_without_improvment++) {
        // Check stop criteria.
        if (parameters.iteration_limit != -1
                && iterations > parameters.iteration_limit)
            break;
        if (parameters.iteration_without_improvment_limit != -1
                && iterations_without_improvment > parameters.iteration_without_improvment_limit)
            break;

        while (solution.feasible()) {
            // Update best solution
            if (output.solution.vertex_number() < solution.vertex_number()) {
                std::stringstream ss;
                ss << "thread " << thread_id
                    << ", it " << iterations
                    << " (" << iterations_without_improvment << ")";
                output.update_solution(solution, ss, parameters.info);
            }

            // Update statistics
            iterations_without_improvment = 0;

        }


        // Update penalties: we increment the penalty of each uncovered element.
        // "reduce" becomes true if we divide by 2 all penalties to avoid
        // integer overflow (this very rarely occur in practice).
        bool reduce = false;
        for (auto it_e = solution.edges().begin(2); it_e != solution.edges().end(2); ++it_e) {
            solution.increment_penalty(*it_e);
            if (solution.penalty(*it_e) > std::numeric_limits<Weight>::max() / instance.edge_number())
                reduce = true;
        }
        if (reduce) {
            //std::cout << "reduce" << std::endl;
            for (EdgeId e = 0; e < instance.edge_number(); ++e)
                solution.set_penalty(e, (solution.penalty(e) - 1) / 2 + 1);
        }
    }
}

LocalSearch1Output stablesolver::localsearch_1(
        const Instance& instance,
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
        if (solution.contains(instance.edge(e).v1))
            vertices[instance.edge(e).v1].score += solution.penalty(e);
        if (solution.contains(instance.edge(e).v2))
            vertices[instance.edge(e).v2].score += solution.penalty(e);
    }
    VertexId v_last_removed = -1;
    VertexId v_last_added = -1;

    Counter iterations_without_improvment = 0;
    for (Counter iterations = 1; parameters.info.check_time(); ++iterations, iterations_without_improvment++) {

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
            assert(solution.penalty() == tmp + score_best);
            // Update scores.
            for (const auto& edge: instance.vertex(v_best).edges) {
                if (solution.covers(edge.e) == 2) {
                    vertices[edge.v].score += solution.penalty(edge.e);
                } else if (solution.covers(edge.e) == 1) {
                    vertices[v_best].score += solution.penalty(edge.e);
                }
            }
            // Update vertices
            vertices[v_best].timestamp = iterations;
            vertices[v_best].last_addition = iterations;
            // Update tabu
            v_last_removed = -1;
            v_last_added   = -1;
            //std::cout << "it " << output.iterations
                //<< " s_best " << s_best
                //<< " p " << solution.penalty()
                //<< " e " << instance.element_number() - solution.element_number() << "/" << instance.element_number()
                //<< " s " << solution.set_number()
                //<< std::endl;
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
        assert(solution.penalty() == tmp1 + score1_best);
        // Update scores.
        for (const auto& edge: instance.vertex(v1_best).edges) {
            if (solution.covers(edge.e) == 2) {
                vertices[edge.v].score += solution.penalty(edge.e);
            } else if (solution.covers(edge.e) == 1) {
                vertices[v1_best].score += solution.penalty(edge.e);
            }
        }
        // Update sets
        vertices[v1_best].timestamp = iterations;
        vertices[v1_best].last_addition = iterations;
        // Update tabu
        v_last_added = v1_best;
        //std::cout << "it " << iterations
            //<< " s1_best " << s1_best
            //<< " score " << score1_best
            //<< " p " << solution.penalty()
            //<< " e " << instance.element_number() - solution.element_number() << "/" << instance.element_number()
            //<< " s " << solution.set_number()
            //<< std::endl;

        // Draw randomly an uncovered element e.
        std::uniform_int_distribution<EdgeId> d_e(
                0, solution.edges().element_number(2) - 1);
        EdgeId e = *(solution.edges().begin(2) + d_e(generator));
        //std::cout << "it " << output.iterations
            //<< " e " << e
            //<< " s " << instance.element(e).sets.size()
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
        assert(solution.penalty() == tmp2 - score2_best);
        // Update scores.
        for (const auto& edge: instance.vertex(v2_best).edges) {
            if (solution.covers(e) == 1) {
                vertices[v2_best].score -= solution.penalty(e);
            } else if (solution.covers(e) == 0) {
                vertices[edge.v].score -= solution.penalty(e);
            }
        }
        // Update sets
        vertices[v2_best].timestamp = iterations;
        vertices[v2_best].last_removal  = iterations;
        vertices[v2_best].iterations += (iterations - vertices[v2_best].last_addition);
        // Update tabu
        v_last_removed = v2_best;
        //std::cout << "it " << iterations
            //<< " s2_best " << s2_best
            //<< " score " << score2_best
            //<< " p " << solution.penalty()
            //<< " e " << instance.element_number() - solution.element_number() << "/" << instance.element_number()
            //<< " s " << solution.set_number()
            //<< std::endl;

        // Update penalties: we increment the penalty of each uncovered element.
        for (auto it = solution.edges().begin(2); it != solution.edges().end(2); ++it) {
            solution.increment_penalty(*it);
            vertices[instance.edge(e).v1].score++;
            vertices[instance.edge(e).v2].score++;
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

