#pragma once

#include "cliquesolver/solution.hpp"

#include "cliquesolver/algorithms/greedy.hpp"
#include "cliquesolver/algorithms/localsearch.hpp"
#include "cliquesolver/algorithms/branchandcut_cplex.hpp"

namespace cliquesolver
{

Output run(std::string algorithm, const Instance& instance, std::mt19937_64& generator, Info info);

}

