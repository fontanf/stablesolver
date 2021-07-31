#pragma once

#include "stablesolver/instance.hpp"

#include "optimizationtools/indexed_set.hpp"
#include "optimizationtools/indexed_map.hpp"

#include <functional>

namespace stablesolver
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
    inline VertexId number_of_vertices() const { return vertices_.size(); }
    inline Weight weight() const { return weight_; }
    inline Weight weight(ComponentId c) const { return component_weights_[c]; }
    inline Weight penalty(EdgeId e) const { return penalties_[e]; }
    inline Weight penalty() const { return penalty_; }
    inline int8_t contains(VertexId v) const { return vertices_.contains(v); }
    inline int8_t covers(EdgeId e) const { return edges_[e]; }
    inline bool feasible() const { return (edges_.number_of_elements(2) == 0); }
    inline bool feasible(ComponentId c) const { return component_number_of_conflictss_[c] == 0; }

    const optimizationtools::IndexedSet& vertices() const { return vertices_; };
    const optimizationtools::DoublyIndexedMap& edges() const { return edges_; }

    inline void add(VertexId v);
    inline void remove(VertexId v);

    void increment_penalty(EdgeId e, Weight p = 1);
    void set_penalty(EdgeId e, Weight p);

    void write(std::string filepath);

private:

    const Instance& instance_;

    optimizationtools::IndexedSet vertices_;
    optimizationtools::DoublyIndexedMap edges_;
    std::vector<EdgeId> component_number_of_conflictss_;
    std::vector<Weight> component_weights_;
    std::vector<Weight> penalties_;
    Weight weight_ = 0;
    Weight penalty_ = 0;

};

void Solution::add(VertexId v)
{
    assert(v >= 0);
    assert(v < instance().number_of_vertices());
    assert(!contains(v));
    ComponentId c = instance().vertex(v).component;
    vertices_.add(v);
    for (const auto& edge: instance().vertex(v).edges) {
        edges_.set(edge.e, edges_[edge.e] + 1);
        if (covers(edge.e) == 2) {
            penalty_ += penalties_[edge.e];
            component_number_of_conflictss_[c]++;
        }
        assert(covers(edge.e) < 2 || (contains(instance().edge(edge.e).v1) && contains(instance().edge(edge.e).v2)));
    }
    weight_               += instance().vertex(v).weight;
    component_weights_[c] += instance().vertex(v).weight;
}

void Solution::remove(VertexId v)
{
    assert(v >= 0);
    assert(v < instance().number_of_vertices());
    assert(contains(v));
    ComponentId c = instance().vertex(v).component;
    for (const auto& edge: instance().vertex(v).edges) {
        if (covers(edge.e) == 2) {
            penalty_ -= penalties_[edge.e];
            component_number_of_conflictss_[c]--;
        }
        edges_.set(edge.e, edges_[edge.e] - 1);
        assert(covers(edge.e) < 2 || (contains(instance().edge(edge.e).v1) && contains(instance().edge(edge.e).v2)));
    }
    vertices_.remove(v);
    weight_               -= instance().vertex(v).weight;
    component_weights_[c] -= instance().vertex(v).weight;
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

    void update_solution(const Solution& solution_new, const std::stringstream& s, Info& info) { update_solution(solution_new, -1, s, info); }
    void update_solution(const Solution& solution_new, ComponentId c, const std::stringstream& s, Info& info);
    void update_upper_bound(Weight upper_bound_new, const std::stringstream& s, Info& info);

    Output& algorithm_end(Info& info);
};

Weight algorithm_end(Weight upper_bound, Info& info);

}

