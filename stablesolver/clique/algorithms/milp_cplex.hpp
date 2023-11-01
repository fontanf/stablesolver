#pragma once

#if CPLEX_FOUND

#include "stablesolver/clique/solution.hpp"

namespace stablesolver
{
namespace clique
{

struct MilpCplexOptionalParameters
{
    optimizationtools::Info info = optimizationtools::Info();

    const Solution* initial_solution = NULL;
};

Output milp_cplex(
        const Instance& instance,
        MilpCplexOptionalParameters parameters = {});

}
}

#endif

