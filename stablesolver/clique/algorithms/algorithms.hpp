#pragma once

#include "stablesolver/clique/solution.hpp"

#include "stablesolver/clique/algorithms/greedy.hpp"
#include "stablesolver/clique/algorithms/milp_cplex.hpp"
#include "stablesolver/clique/algorithms/local_search.hpp"

namespace stablesolver
{
namespace clique
{

Output run(
        std::string algorithm,
        const Instance& instance,
        std::mt19937_64& generator,
        optimizationtools::Info info);

}
}

