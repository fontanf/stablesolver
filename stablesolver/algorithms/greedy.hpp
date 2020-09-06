#pragma once

#include "stablesolver/solution.hpp"

namespace stablesolver
{

Output greedy_gwmin(const Instance& instance, Info info = Info());

Output greedy_gwmax(const Instance& instance, Info info = Info());

Output greedy_gwmin2(const Instance& instance, Info info = Info());

}

