#pragma once

#include "cliquesolver/solution.hpp"

#include "cliquesolver/algorithms/greedy.hpp"
#include "cliquesolver/algorithms/milp_cplex.hpp"
#include "cliquesolver/algorithms/local_search.hpp"

namespace cliquesolver
{

Output run(
        std::string algorithm,
        const Instance& instance,
        std::mt19937_64& generator,
        optimizationtools::Info info);

}

