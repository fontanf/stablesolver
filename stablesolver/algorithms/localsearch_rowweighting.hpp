#pragma once

#include "stablesolver/solution.hpp"

namespace stablesolver
{

/******************************* localsearch_rowweighting_1 ********************************/

struct LocalSearchRowWeighting1OptionalParameters
{
    Counter thread_number = 3;
    Counter iteration_limit = -1;
    Counter iteration_without_improvment_limit = -1;
    Info info = Info();
};

struct LocalSearchRowWeighting1Output: Output
{
    LocalSearchRowWeighting1Output(Instance& instance, Info& info): Output(instance, info) { }
    LocalSearchRowWeighting1Output& algorithm_end(Info& info);
};

LocalSearchRowWeighting1Output localsearch_rowweighting_1(
        Instance& instance,
        std::mt19937_64& generator,
        LocalSearchRowWeighting1OptionalParameters parameters = {});

/******************************* localsearch_rowweighting_2 ********************************/

struct LocalSearchRowWeighting2OptionalParameters
{
    Counter thread_number = 3;
    Counter iteration_limit = -1;
    Counter iteration_without_improvment_limit = -1;
    Info info = Info();
};

struct LocalSearchRowWeighting2Output: Output
{
    LocalSearchRowWeighting2Output(const Instance& instance, Info& info): Output(instance, info) { }
    LocalSearchRowWeighting2Output& algorithm_end(Info& info);
};

LocalSearchRowWeighting2Output localsearch_rowweighting_2(
        const Instance& instance,
        std::mt19937_64& generator,
        LocalSearchRowWeighting2OptionalParameters parameters = {});

}

