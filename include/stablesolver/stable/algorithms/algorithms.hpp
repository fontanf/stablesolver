#pragma once

#include "stablesolver/stable/solution.hpp"

#include "stablesolver/stable/algorithms/greedy.hpp"
#include "stablesolver/stable/algorithms/milp_cbc.hpp"
#include "stablesolver/stable/algorithms/milp_cplex.hpp"
#include "stablesolver/stable/algorithms/local_search.hpp"
#include "stablesolver/stable/algorithms/local_search_row_weighting.hpp"
#include "stablesolver/stable/algorithms/large_neighborhood_search.hpp"

namespace stablesolver
{
namespace stable
{

Output run(
        std::string algorithm,
        Instance& instance,
        std::mt19937_64& generator,
        optimizationtools::Info info);

}
}

