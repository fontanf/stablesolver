#pragma once

#include "stablesolver/solution.hpp"

#include "stablesolver/algorithms/greedy.hpp"
#include "stablesolver/algorithms/milp_cbc.hpp"
#include "stablesolver/algorithms/milp_cplex.hpp"
#include "stablesolver/algorithms/local_search.hpp"
#include "stablesolver/algorithms/local_search_row_weighting.hpp"
#include "stablesolver/algorithms/large_neighborhood_search.hpp"

namespace stablesolver
{

Output run(
        std::string algorithm,
        Instance& instance,
        std::mt19937_64& generator,
        optimizationtools::Info info);

}

