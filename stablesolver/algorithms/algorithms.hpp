#pragma once

#include "stablesolver/solution.hpp"

#include "stablesolver/algorithms/greedy.hpp"
#include "stablesolver/algorithms/branchandcut_cplex.hpp"

namespace stablesolver
{

Output run(
        std::string algorithm,
        const Instance& instance,
        std::mt19937_64& generator,
        Info info);

}

