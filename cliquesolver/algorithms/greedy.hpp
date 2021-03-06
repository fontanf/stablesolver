#pragma once

#include "cliquesolver/solution.hpp"

namespace cliquesolver
{

Output greedy_gwmin(const Instance& instance, Info info = Info());

Output greedy_gwmax(const Instance& instance, Info info = Info());

Output greedy_gwmin2(const Instance& instance, Info info = Info());

}

