#pragma once

#include "stablesolver/solution.hpp"

namespace stablesolver
{

struct LargeNeighborhoodSearchOptionalParameters
{
    Info info = Info();

    Counter thread_number = 3;
    Counter iteration_limit = -1;
    Counter iteration_without_improvment_limit = -1;
};

struct LargeNeighborhoodSearchOutput: Output
{
    LargeNeighborhoodSearchOutput(const Instance& instance, Info& info): Output(instance, info) { }
    LargeNeighborhoodSearchOutput& algorithm_end(Info& info);

    Counter iterations = 0;
};

LargeNeighborhoodSearchOutput largeneighborhoodsearch(
        Instance& instance,
        LargeNeighborhoodSearchOptionalParameters parameters = {});

}

