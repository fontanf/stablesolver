#pragma once

#include "stablesolver/solution.hpp"

namespace stablesolver
{

struct LargeNeighborhoodSearchOptionalParameters
{
    optimizationtools::Info info = optimizationtools::Info();

    Counter number_of_threads = 3;
    Counter maximum_number_of_iterations = -1;
    Counter maximum_number_of_iterations_without_improvement = -1;
};

struct LargeNeighborhoodSearchOutput: Output
{
    LargeNeighborhoodSearchOutput(
            const Instance& instance,
            optimizationtools::Info& info):
        Output(instance, info) { }

    LargeNeighborhoodSearchOutput& algorithm_end(
            optimizationtools::Info& info);

    Counter iterations = 0;
};

LargeNeighborhoodSearchOutput largeneighborhoodsearch(
        Instance& instance,
        LargeNeighborhoodSearchOptionalParameters parameters = {});

}

