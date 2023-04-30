#pragma once

#include "stablesolver/solution.hpp"

namespace stablesolver
{

struct GreedyOptionalParameters
{
    /** Reduction parameters. */
    ReductionParameters reduction_parameters;

    /** Info structure. */
    optimizationtools::Info info = optimizationtools::Info();
};

Output greedy_gwmin(
        const Instance& instance,
        GreedyOptionalParameters parameters = {});

Output greedy_gwmax(
        const Instance& instance,
        GreedyOptionalParameters parameters = {});

Output greedy_gwmin2(
        const Instance& instance,
        GreedyOptionalParameters parameters = {});

Output greedy_strong(
        const Instance& instance,
        GreedyOptionalParameters parameters = {});

}

