#pragma once

#if CPLEX_FOUND

#include "cliquesolver/solution.hpp"

namespace cliquesolver
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

#endif

