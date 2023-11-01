#pragma once

#if CPLEX_FOUND

#include "stablesolver/stable/solution.hpp"

namespace stablesolver
{
namespace stable
{

struct MilpCplexOptionalParameters
{
    /** Initial solution. */
    const Solution* initial_solution = NULL;

    /** Reduction parameters. */
    ReductionParameters reduction_parameters;

    /** Info structure. */
    optimizationtools::Info info = optimizationtools::Info();
};

Output milp_1_cplex(
        const Instance& instance,
        MilpCplexOptionalParameters parameters = {});

Output milp_2_cplex(
        const Instance& instance,
        MilpCplexOptionalParameters parameters = {});

Output milp_3_cplex(
        const Instance& instance,
        MilpCplexOptionalParameters parameters = {});

}
}

#endif

