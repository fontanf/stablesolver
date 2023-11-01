#pragma once

#include "stablesolver/clique/solution.hpp"

namespace stablesolver
{
namespace clique
{

Output greedy_gwmin(
        const Instance& instance,
        optimizationtools::Info info = optimizationtools::Info());

Output greedy_strong(
        const Instance& instance,
        optimizationtools::Info info = optimizationtools::Info());

}
}

