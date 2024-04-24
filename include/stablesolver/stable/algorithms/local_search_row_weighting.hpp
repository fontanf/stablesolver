#pragma once

#include "stablesolver/stable/algorithm.hpp"

namespace stablesolver
{
namespace stable
{

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct LocalSearchRowWeighting1Parameters: Parameters
{
    /** Maximum number of iterations. */
    Counter maximum_number_of_iterations = -1;

    /** Maximum number of iterations without improvement. */
    Counter maximum_number_of_iterations_without_improvement = -1;
};

struct LocalSearchRowWeighting1Output: Output
{
    LocalSearchRowWeighting1Output(
            const Instance& instance):
        Output(instance) { }


    /** Number of iterations. */
    Counter number_of_iterations = 0;
};

const LocalSearchRowWeighting1Output local_search_row_weighting_1(
        const Instance& instance,
        std::mt19937_64& generator,
        const LocalSearchRowWeighting1Parameters& parameters = {});

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct LocalSearchRowWeighting2Parameters: Parameters
{
    /** Maximum number of iterations. */
    Counter maximum_number_of_iterations = -1;

    /** Maximum number of iterations without improvement. */
    Counter maximum_number_of_iterations_without_improvement = -1;
};

struct LocalSearchRowWeighting2Output: Output
{
    LocalSearchRowWeighting2Output(
            const Instance& instance):
        Output(instance) { }


    /** Number of iterations. */
    Counter number_of_iterations = 0;
};

const LocalSearchRowWeighting2Output local_search_row_weighting_2(
        const Instance& instance,
        std::mt19937_64& generator,
        const LocalSearchRowWeighting2Parameters& parameters = {});

}
}
