#pragma once

#if CPLEX_FOUND

#include "stablesolver/solution.hpp"

namespace stablesolver
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

MilpCplexOutput milp_1_cplex(
        const Instance& instance, MilpCplexOptionalParameters parameters = {});
MilpCplexOutput milp_2_cplex(
        const Instance& instance, MilpCplexOptionalParameters parameters = {});
MilpCplexOutput milp_3_cplex(
        const Instance& instance, MilpCplexOptionalParameters parameters = {});

}

#endif

