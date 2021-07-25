#pragma once

#if CPLEX_FOUND

#include "stablesolver/solution.hpp"

namespace stablesolver
{

struct MilpCplexOptionalParameters
{
    Info info = Info();

    const Solution* initial_solution = NULL;
};

struct MilpCplexOutput: Output
{
    MilpCplexOutput(const Instance& instance, Info& info): Output(instance, info) { }
    MilpCplexOutput& algorithm_end(Info& info);
};

MilpCplexOutput milp_1_cplex(
        const Instance& instance, MilpCplexOptionalParameters parameters = {});
MilpCplexOutput milp_2_cplex(
        const Instance& instance, MilpCplexOptionalParameters parameters = {});
MilpCplexOutput milp_3_cplex(
        const Instance& instance, MilpCplexOptionalParameters parameters = {});

}

#endif

