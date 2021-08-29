#pragma once

#include "stablesolver/solution.hpp"

namespace stablesolver
{

/******************************* localsearch_rowweighting_1 ********************************/

struct LocalSearchRowWeighting1OptionalParameters
{
    Counter number_of_threads = 3;
    Counter maximum_number_of_iterations = -1;
    Counter maximum_number_of_iterations_without_improvement = -1;
    optimizationtools::Info info = optimizationtools::Info();
};

struct LocalSearchRowWeighting1Output: Output
{
    LocalSearchRowWeighting1Output(
            Instance& instance,
            optimizationtools::Info& info):
        Output(instance, info) { }

    LocalSearchRowWeighting1Output& algorithm_end(
            optimizationtools::Info& info);
};

LocalSearchRowWeighting1Output localsearch_rowweighting_1(
        Instance& instance,
        std::mt19937_64& generator,
        LocalSearchRowWeighting1OptionalParameters parameters = {});

/******************************* localsearch_rowweighting_2 ********************************/

struct LocalSearchRowWeighting2OptionalParameters
{
    Counter number_of_threads = 3;
    Counter maximum_number_of_iterations = -1;
    Counter maximum_number_of_iterations_without_improvement = -1;
    optimizationtools::Info info = optimizationtools::Info();
};

struct LocalSearchRowWeighting2Output: Output
{
    LocalSearchRowWeighting2Output(
            const Instance& instance,
            optimizationtools::Info& info):
        Output(instance, info) { }

    LocalSearchRowWeighting2Output& algorithm_end(
            optimizationtools::Info& info);
};

LocalSearchRowWeighting2Output localsearch_rowweighting_2(
        const Instance& instance,
        std::mt19937_64& generator,
        LocalSearchRowWeighting2OptionalParameters parameters = {});

}

