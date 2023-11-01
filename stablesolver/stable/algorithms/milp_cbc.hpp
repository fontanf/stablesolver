#pragma once

#if COINOR_FOUND

#include "stablesolver/stable/solution.hpp"

namespace stablesolver
{
namespace stable
{

struct MilpCbcOptionalParameters
{
    /** Initial solution. */
    const Solution* initial_solution = NULL;

    /** Reduction parameters. */
    ReductionParameters reduction_parameters;

    /** Info structure. */
    optimizationtools::Info info = optimizationtools::Info();
};

Output milp_1_cbc(
        const Instance& instance,
        MilpCbcOptionalParameters parameters = {});

}
}

#endif

