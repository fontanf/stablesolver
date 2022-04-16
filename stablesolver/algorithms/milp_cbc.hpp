#pragma once

#if COINOR_FOUND

#include "stablesolver/solution.hpp"

namespace stablesolver
{

struct MilpCbcOptionalParameters
{
    optimizationtools::Info info = optimizationtools::Info();

    const Solution* initial_solution = NULL;
};

struct MilpCbcOutput: Output
{
    MilpCbcOutput(
            const Instance& instance,
            optimizationtools::Info& info):
        Output(instance, info) { }

    MilpCbcOutput& algorithm_end(
            optimizationtools::Info& info);
};

MilpCbcOutput milp_1_cbc(
        const Instance& instance,
        MilpCbcOptionalParameters parameters = {});

}

#endif

