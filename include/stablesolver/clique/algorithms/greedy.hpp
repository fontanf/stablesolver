#pragma once

#include "stablesolver/clique/solution.hpp"

namespace stablesolver
{
namespace clique
{

const Output greedy_gwmin(
        const Instance& instance,
        const Parameters& parameters = {});

const Output greedy_strong(
        const Instance& instance,
        const Parameters& parameters = {});

}
}
