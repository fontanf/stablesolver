#pragma once

#include "cliquesolver/solution.hpp"

#include "cliquesolver/algorithms/greedy.hpp"

namespace cliquesolver
{

Output run(
        std::string algorithm,
        const Instance& instance,
        std::mt19937_64& generator,
        optimizationtools::Info info);

}

