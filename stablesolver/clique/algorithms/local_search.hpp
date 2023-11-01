/**
 * Local Search:
 *
 */

#pragma once

#include "stablesolver/clique/solution.hpp"

namespace stablesolver
{
namespace clique
{

struct LocalSearchOptionalParameters
{
    optimizationtools::Info info = optimizationtools::Info();

    /** Maximum number of nodes. */
    Counter maximum_number_of_nodes = -1;

    /** Number of threads. */
    Counter number_of_threads = 1;
};

Output local_search(
        const Instance& instance,
        std::mt19937_64& generator,
        LocalSearchOptionalParameters parameters = {});

}
}

