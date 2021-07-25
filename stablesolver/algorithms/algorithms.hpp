#pragma once

#include "stablesolver/solution.hpp"

#include "stablesolver/algorithms/greedy.hpp"
#include "stablesolver/algorithms/milp_cplex.hpp"
#include "stablesolver/algorithms/localsearch.hpp"
#include "stablesolver/algorithms/localsearch_rowweighting.hpp"
#include "stablesolver/algorithms/largeneighborhoodsearch.hpp"

namespace stablesolver
{

Output run(
        std::string algorithm,
        Instance& instance,
        std::mt19937_64& generator,
        Info info);

}

