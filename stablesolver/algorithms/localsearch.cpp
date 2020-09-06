#include "stablesolver/algorithms/localsearch.hpp"

#include "stablesolver/algorithms/greedy.hpp"

#include <thread>

using namespace stablesolver;

LocalSearchOutput& LocalSearchOutput::algorithm_end(Info& info)
{
    //PUT(info, "Algorithm", "Iterations", iterations);
    Output::algorithm_end(info);
    //VER(info, "Iterations: " << iterations << std::endl);
    return *this;
}

struct LocalSearchVertex
{
    Counter timestamp = -1;
};

void localsearch_worker(
        const Instance& instance,
        Seed seed,
        LocalSearchOptionalParameters parameters,
        LocalSearchOutput& output,
        Counter thread_id)
{
    std::mt19937_64 generator(seed);
    parameters.info.output->mutex_sol.lock();
    Solution solution = output.solution;
    parameters.info.output->mutex_sol.unlock();

    // Initialize local search structures.
    std::vector<LocalSearchVertex> vertices(instance.vertex_number());
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
        for (auto it_e = solution.conflicts().begin(); it_e != solution.conflicts().end(); ++it_e) {
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

LocalSearchOutput stablesolver::localsearch(
        const Instance& instance,
        std::mt19937_64& generator,
        LocalSearchOptionalParameters parameters)
{
    VER(parameters.info, "*** localsearch ***" << std::endl);

    // Compute initial greedy solution.
    LocalSearchOutput output(instance, parameters.info);
    Solution solution = greedy_gwmin(instance).solution;
    std::stringstream ss;
    ss << "initial solution";
    output.update_solution(solution, ss, parameters.info);

    auto seeds = optimizationtools::bob_floyd(parameters.thread_number, std::numeric_limits<Seed>::max(), generator);
    std::vector<std::thread> threads;
    for (Counter thread_id = 0; thread_id < parameters.thread_number; ++thread_id)
        threads.push_back(std::thread(localsearch_worker, std::ref(instance), seeds[thread_id], parameters, std::ref(output), thread_id));

    for (Counter thread_id = 0; thread_id < parameters.thread_number; ++thread_id)
        threads[thread_id].join();

    return output.algorithm_end(parameters.info);
}

