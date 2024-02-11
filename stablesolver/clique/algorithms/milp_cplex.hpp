#pragma once

#if CPLEX_FOUND

#include "stablesolver/clique/solution.hpp"

namespace stablesolver
{
namespace clique
{

struct MilpCplexParameters: Parameters
{
    const Solution* initial_solution = NULL;
};

const Output milp_cplex(
        const Instance& instance,
        const MilpCplexParameters& parameters = {});

}
}

#endif
