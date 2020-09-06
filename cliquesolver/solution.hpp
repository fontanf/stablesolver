#pragma once

#include "cliquesolver/instance.hpp"

#include "optimizationtools/indexed_set.hpp"
#include "optimizationtools/indexed_map.hpp"

#include <functional>

namespace cliquesolver
{

class Solution
{

public:

    Solution(const Instance& instance);
    Solution(const Instance& instance, std::string filepath);
    Solution(const Solution& solution);
    Solution& operator=(const Solution& solution);
    ~Solution() { }
    bool operator==(const Solution& solution);

    inline const Instance& instance() const { return instance_; }
    inline VertexId vertex_number() const { return vertices_.size(); }
    inline Weight weight() const { return weight_; }
    inline Weight penalty() const { return penalty_; }
    inline int8_t contains(VertexId e) const { return vertices_.contains(e); }
    inline bool feasible() const { return penalty() == 0; }

    const optimizationtools::IndexedSet& vertices() const { return vertices_; };

    inline void add(VertexId v);
    inline void remove(VertexId v);

    void write_cert(std::string filepath);

private:

    const Instance& instance_;

    optimizationtools::IndexedSet vertices_;
    optimizationtools::IndexedSet neighbors_tmp_;
    Weight weight_ = 0;
    Weight penalty_ = 0;

};

void Solution::add(VertexId v)
{
    assert(v >= 0);
    assert(v < instance().vertex_number());
    assert(!contains(v));
    neighbors_tmp_.clear();
    for (const auto& edge: instance().vertex(v).edges)
        neighbors_tmp_.add(edge.v);
    for (VertexId v2: vertices())
        if (!neighbors_tmp_.contains(v2))
            penalty_ += 1;
    vertices_.add(v);
    weight_ += instance().vertex(v).weight;
}

void Solution::remove(VertexId v)
{
    assert(v >= 0);
    assert(v < instance().vertex_number());
    assert(contains(v));
    vertices_.remove(v);
    weight_ -= instance().vertex(v).weight;
    neighbors_tmp_.clear();
    for (const auto& edge: instance().vertex(v).edges)
        neighbors_tmp_.add(edge.v);
    for (VertexId v2: vertices())
        if (!neighbors_tmp_.contains(v2))
            penalty_ -= 1;
}

std::ostream& operator<<(std::ostream& os, const Solution& solution);

/*********************************** Output ***********************************/

struct Output
{
    Output(const Instance& instance, Info& info);
    Solution solution;
    Weight upper_bound = 0;
    double time = -1;

    bool optimal() const { return solution.feasible() && solution.weight() == upper_bound; }
    Weight lower_bound() const { return solution.weight(); }
    double gap() const;
    void print(Info& info, const std::stringstream& s) const;

    void update_solution(const Solution& solution_new, const std::stringstream& s, Info& info);
    void update_upper_bound(Weight upper_bound_new, const std::stringstream& s, Info& info);

    Output& algorithm_end(Info& info);
};

Weight algorithm_end(Weight upper_bound, Info& info);

}

