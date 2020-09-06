#pragma once

#include "stablesolver/solution.hpp"

namespace stablesolver
{

struct LocalSearchOptionalParameters
{
    Counter thread_number = 3;
    Counter iteration_limit = -1;
    Counter iteration_without_improvment_limit = -1;
    Info info = Info();
};

struct LocalSearchOutput: Output
{
    LocalSearchOutput(const Instance& instance, Info& info): Output(instance, info) { }
    LocalSearchOutput& algorithm_end(Info& info);
};

LocalSearchOutput localsearch(
        const Instance& instance,
        std::mt19937_64& generator,
        LocalSearchOptionalParameters parameters = {});

}

