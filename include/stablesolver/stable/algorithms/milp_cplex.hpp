#pragma once

#include "stablesolver/stable/algorithm.hpp"

namespace stablesolver
{
namespace stable
{

struct MilpCplexParameters: Parameters
{
    /** Initial solution. */
    const Solution* initial_solution = NULL;
};

const Output milp_1_cplex(
        const Instance& instance,
        const MilpCplexParameters& parameters = {});

const Output milp_2_cplex(
        const Instance& instance,
        const MilpCplexParameters& parameters = {});

const Output milp_3_cplex(
        const Instance& instance,
        const MilpCplexParameters& parameters = {});

}
}
