#pragma once

#include "stablesolver/solution.hpp"

namespace stablesolver
{

/******************************* localsearch_1 ********************************/

struct LocalSearch1OptionalParameters
{
    Counter thread_number = 3;
    Counter iteration_limit = -1;
    Counter iteration_without_improvment_limit = -1;
    Info info = Info();
};

struct LocalSearch1Output: Output
{
    LocalSearch1Output(Instance& instance, Info& info): Output(instance, info) { }
    LocalSearch1Output& algorithm_end(Info& info);
};

LocalSearch1Output localsearch_1(
        Instance& instance,
        std::mt19937_64& generator,
        LocalSearch1OptionalParameters parameters = {});

/******************************* localsearch_2 ********************************/

struct LocalSearch2OptionalParameters
{
    Counter thread_number = 3;
    Counter iteration_limit = -1;
    Counter iteration_without_improvment_limit = -1;
    Info info = Info();
};

struct LocalSearch2Output: Output
{
    LocalSearch2Output(const Instance& instance, Info& info): Output(instance, info) { }
    LocalSearch2Output& algorithm_end(Info& info);
};

LocalSearch2Output localsearch_2(
        const Instance& instance,
        std::mt19937_64& generator,
        LocalSearch2OptionalParameters parameters = {});

}

