#pragma once

#include "stablesolver/solution.hpp"

namespace stablesolver
{

Output greedy_gwmin(
        const Instance& instance,
        optimizationtools::Info info = optimizationtools::Info());

Output greedy_gwmax(
        const Instance& instance,
        optimizationtools::Info info = optimizationtools::Info());

Output greedy_gwmin2(
        const Instance& instance,
        optimizationtools::Info info = optimizationtools::Info());

Output greedy_strong(
        const Instance& instance,
        optimizationtools::Info info = optimizationtools::Info());

}

