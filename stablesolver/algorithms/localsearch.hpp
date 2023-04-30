/**
 * Local Search:
 *
 * Two neighborhoods are implemented:
 * - Add: add a vertex to the solution and remove its neighbors.
 *   Complexity: O(n)
 * - (2-1)-swap: remove an item from the knapsack and add two of its neighbors
 *   which are not neighbors.
 *   Complexity: O(number of edges)
 *   See:
 *   - "Fast local search for the maximum independent set problem" (Andrade et
 *     al., 2012)
 *     https://doi.org/10.1007/s10732-012-9196-4
 *   - "A hybrid iterated local search heuristic for the maximum weight
 *     independent set problem" (Nogueira et al., 2018)
 *     https://doi.org/10.1007/s11590-017-1128-7
 *
 */

#pragma once

#include "stablesolver/solution.hpp"

namespace stablesolver
{

struct LocalSearchOptionalParameters
{
    /** Maximum number of nodes. */
    Counter maximum_number_of_nodes = -1;

    /** Number of threads. */
    Counter number_of_threads = 1;

    /** Reduction parameters. */
    ReductionParameters reduction_parameters;

    /** Info structure. */
    optimizationtools::Info info = optimizationtools::Info();
};

struct LocalSearchOutput: Output
{
    LocalSearchOutput(
            const Instance& instance,
            optimizationtools::Info& info):
        Output(instance, info) { }

    LocalSearchOutput& algorithm_end(
            optimizationtools::Info& info);
};

LocalSearchOutput localsearch(
        const Instance& instance,
        std::mt19937_64& generator,
        LocalSearchOptionalParameters parameters = {});

}

