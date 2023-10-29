#pragma once

#include "stablesolver/solution.hpp"

namespace stablesolver
{

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct LocalSearchRowWeighting1Output: Output
{
    LocalSearchRowWeighting1Output(
            const Instance& instance,
            optimizationtools::Info& info):
        Output(instance, info) { }

    void print_statistics(
            optimizationtools::Info& info) const override;

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

LocalSearchRowWeighting1Output local_search_row_weighting_1(
        const Instance& instance,
        std::mt19937_64& generator,
        LocalSearchRowWeighting1OptionalParameters parameters = {});

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct LocalSearchRowWeighting2Output: Output
{
    LocalSearchRowWeighting2Output(
            const Instance& instance,
            optimizationtools::Info& info):
        Output(instance, info) { }

    void print_statistics(
            optimizationtools::Info& info) const override;

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

LocalSearchRowWeighting2Output local_search_row_weighting_2(
        const Instance& instance,
        std::mt19937_64& generator,
        LocalSearchRowWeighting2OptionalParameters parameters = {});

}

