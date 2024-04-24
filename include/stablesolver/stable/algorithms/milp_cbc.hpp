#pragma once

#include "stablesolver/stable/algorithm.hpp"

namespace stablesolver
{
namespace stable
{

struct MilpCbcParameters: Parameters
{
    /** Initial solution. */
    const Solution* initial_solution = NULL;
};

const Output milp_1_cbc(
        const Instance& instance,
        const MilpCbcParameters& parameters = {});

}
}
