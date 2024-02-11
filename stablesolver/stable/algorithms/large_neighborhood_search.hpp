#pragma once

#include "stablesolver/stable/algorithm.hpp"

namespace stablesolver
{
namespace stable
{

struct LargeNeighborhoodSearchParameters: Parameters
{
    /** Number of threads. */
    Counter number_of_threads = 3;

    /** Maximum number of iterations. */
    Counter maximum_number_of_iterations = -1;

    /** Maximum number of iterations without improvement. */
    Counter maximum_number_of_iterations_without_improvement = -1;
};

struct LargeNeighborhoodSearchOutput: Output
{
    LargeNeighborhoodSearchOutput(
            const Instance& instance):
        Output(instance) { }


    /** Number of iterations. */
    Counter iterations = 0;
};

const LargeNeighborhoodSearchOutput large_neighborhood_search(
        const Instance& instance,
        const LargeNeighborhoodSearchParameters& parameters = {});

}
}
