#pragma once

#include "stablesolver/solution.hpp"

namespace stablesolver
{

////////////////////////////////////////////////////////////////////////////////
////////////////////////// localsearch_rowweighting_1 //////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct LocalSearchRowWeighting1Output: Output
{
    LocalSearchRowWeighting1Output(
            const Instance& instance,
            optimizationtools::Info& info):
        Output(instance, info) { }

    LocalSearchRowWeighting1Output& algorithm_end(
            optimizationtools::Info& info);

    /** Number of iterations. */
    Counter number_of_iterations = 0;
};

using LocalSearchRowWeighting1Callback = std::function<void(const LocalSearchRowWeighting1Output&)>;

struct LocalSearchRowWeighting1OptionalParameters
{
    /** Maximum number of iterations. */
    Counter maximum_number_of_iterations = -1;

    /** Maximum number of iterations without improvement. */
    Counter maximum_number_of_iterations_without_improvement = -1;

    /** Callback function called when a new best solution is found. */
    LocalSearchRowWeighting1Callback new_solution_callback
        = [](const LocalSearchRowWeighting1Output&) { };

    /** Reduction parameters. */
    ReductionParameters reduction_parameters;

    /** Info structure. */
    optimizationtools::Info info = optimizationtools::Info();
};

LocalSearchRowWeighting1Output localsearch_rowweighting_1(
        const Instance& instance,
        std::mt19937_64& generator,
        LocalSearchRowWeighting1OptionalParameters parameters = {});

////////////////////////////////////////////////////////////////////////////////
////////////////////////// localsearch_rowweighting_2 //////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct LocalSearchRowWeighting2Output: Output
{
    LocalSearchRowWeighting2Output(
            const Instance& instance,
            optimizationtools::Info& info):
        Output(instance, info) { }

    LocalSearchRowWeighting2Output& algorithm_end(
            optimizationtools::Info& info);

    /** Number of iterations. */
    Counter number_of_iterations = 0;
};

using LocalSearchRowWeighting2Callback = std::function<void(const LocalSearchRowWeighting2Output&)>;

struct LocalSearchRowWeighting2OptionalParameters
{
    /** Maximum number of iterations. */
    Counter maximum_number_of_iterations = -1;

    /** Maximum number of iterations without improvement. */
    Counter maximum_number_of_iterations_without_improvement = -1;

    /** Callback function called when a new best solution is found. */
    LocalSearchRowWeighting2Callback new_solution_callback
        = [](const LocalSearchRowWeighting2Output&) { };

    /** Reduction parameters. */
    ReductionParameters reduction_parameters;

    /** Info structure. */
    optimizationtools::Info info = optimizationtools::Info();
};

LocalSearchRowWeighting2Output localsearch_rowweighting_2(
        const Instance& instance,
        std::mt19937_64& generator,
        LocalSearchRowWeighting2OptionalParameters parameters = {});

}

