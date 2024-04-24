#pragma once

#include "stablesolver/stable/algorithm.hpp"

namespace stablesolver
{
namespace stable
{

struct GreedyParameters: Parameters
{
};

const Output greedy_gwmin(
        const Instance& instance,
        const GreedyParameters& parameters = {});

const Output greedy_gwmax(
        const Instance& instance,
        const GreedyParameters& parameters = {});

const Output greedy_gwmin2(
        const Instance& instance,
        const GreedyParameters& parameters = {});

const Output greedy_strong(
        const Instance& instance,
        const GreedyParameters& parameters = {});

}
}
