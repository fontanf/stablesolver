#pragma once

#if CPLEX_FOUND

#include "stablesolver/solution.hpp"

namespace stablesolver
{

struct BranchAndCutCplexOptionalParameters
{
    Info info = Info();

    const Solution* initial_solution = NULL;
};

struct BranchAndCutCplexOutput: Output
{
    BranchAndCutCplexOutput(const Instance& instance, Info& info): Output(instance, info) { }
    BranchAndCutCplexOutput& algorithm_end(Info& info);
};

BranchAndCutCplexOutput branchandcut_1_cplex(
        const Instance& instance, BranchAndCutCplexOptionalParameters parameters = {});
BranchAndCutCplexOutput branchandcut_2_cplex(
        const Instance& instance, BranchAndCutCplexOptionalParameters parameters = {});
BranchAndCutCplexOutput branchandcut_3_cplex(
        const Instance& instance, BranchAndCutCplexOptionalParameters parameters = {});

}

#endif

