#pragma once

#include "cliquesolver/solution.hpp"

namespace cliquesolver
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

}

