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

struct MilpCplexOutput: Output
{
    MilpCplexOutput(
            const Instance& instance,
            optimizationtools::Info& info):
        Output(instance, info) { }

    MilpCplexOutput& algorithm_end(
            optimizationtools::Info& info);
};

MilpCplexOutput milp_cplex(
        const Instance& instance,
        MilpCplexOptionalParameters parameters = {});

}

#endif

