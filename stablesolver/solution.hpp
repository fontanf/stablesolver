#pragma once

#include "stablesolver/instance.hpp"

#include "optimizationtools/indexed_set.hpp"
#include "optimizationtools/indexed_map.hpp"

#include <functional>
#include <unordered_set>

namespace stablesolver
{

class Solution
{

public:

    /*
     * Constructors and destructor.
     */

    /** Create an empty solution. */
    Solution(const Instance& instance);
    /** Create a solution from a certificate file. */
    Solution(const Instance& instance, std::string certificate_path);
    /** Copy constructor. */
    Solution(const Solution& solution);
    /** Copy assignment operator. */
    Solution& operator=(const Solution& solution);
    /** Destructor. */
    ~Solution() { }

    /** Get the instance. */
    inline const Instance& instance() const { return instance_; }
    /** Get the number of vertices of the solution. */
    inline VertexId number_of_vertices() const { return vertices_.size(); }
    /** Get the weight of the solution. */
    inline Weight weight() const { return weight_; }
    /** Get the weight of connected component c. */
    inline Weight weight(ComponentId c) const { return component_weights_[c]; }
    /** Return 'true' iff vertex v is in the solution. */
    inline int8_t contains(VertexId v) const { return vertices_.contains(v); }
    /** Return the number of ends of edge e in the solution. */
    inline int8_t covers(EdgeId e);
    /** Get the number of conflitcs in the solution. */
    EdgePos number_of_conflicts() const { return conflicts_.size(); }
    /** Return 'true' iff the solution is feasible. */
    inline bool feasible() const { return (conflicts_.empty()); }
    /** Return 'true' iff component c is feasible. */
    inline bool feasible(ComponentId c) const { return component_number_of_conflictss_[c] == 0; }
    /** Get the set of vertices of the solution. */
    const optimizationtools::IndexedSet& vertices() const { return vertices_; };
    /** Get the set of edges of the solution. */
    const std::unordered_set<EdgeId>& conflicts() const { return conflicts_; }

    /*
     * Setters.
     */

    /** Add vertex v to the solution. */
    inline void add(VertexId v);
    /** Remove vertex v from the solution. */
    inline void remove(VertexId v);

    /*
     * Export.
     */

    /** Write the solution to a file. */
    void write(std::string certificate_path);

private:

    /** Instance. */
    const Instance& instance_;
    /** Set of vertices of the solution. */
    optimizationtools::IndexedSet vertices_;
    /** Set of conflicting edges. */
    std::unordered_set<EdgeId> conflicts_;
    /** Number of conflicts in each component. */
    std::vector<EdgeId> component_number_of_conflictss_;
    /** Weights of each component. */
    std::vector<Weight> component_weights_;
    /** Weight of the solution. */
    Weight weight_ = 0;

};

int8_t Solution::covers(EdgeId e)
{
    if (contains(instance_.edge(e).v1)) {
        if (contains(instance_.edge(e).v2)) {
            return 2;
        } else {
            return 1;
        }
    } else {
        if (contains(instance_.edge(e).v2)) {
            return 1;
        } else {
            return 0;
        }
    }
}

void Solution::add(VertexId v)
{
    // Checks.
    instance().check_vertex_index(v);
    if (contains(v)) {
        throw std::invalid_argument(
                "Cannot add vertex " + std::to_string(v)
                + " which is already in the solution");
    }

    ComponentId c = instance().vertex(v).component;
    for (const auto& edge: instance().vertex(v).edges) {
        if (covers(edge.e) == 1) {
            component_number_of_conflictss_[c]++;
            conflicts_.insert(edge.e);
        }
    }
    weight_               += instance().vertex(v).weight;
    component_weights_[c] += instance().vertex(v).weight;
    vertices_.add(v);
}

void Solution::remove(VertexId v)
{
    // Checks.
    instance().check_vertex_index(v);
    if (!contains(v)) {
        throw std::invalid_argument(
                "Cannot remove vertex " + std::to_string(v)
                + " which is not in the solution");
    }

    ComponentId c = instance().vertex(v).component;
    for (const auto& edge: instance().vertex(v).edges) {
        if (covers(edge.e) == 2) {
            component_number_of_conflictss_[c]--;
            conflicts_.erase(edge.e);
        }
    }
    weight_               -= instance().vertex(v).weight;
    component_weights_[c] -= instance().vertex(v).weight;
    vertices_.remove(v);
}

std::ostream& operator<<(std::ostream& os, const Solution& solution);

/*********************************** Output ***********************************/

struct Output
{
    Output(
            const Instance& instance,
            optimizationtools::Info& info);

    Solution solution;
    Weight upper_bound = 0;
    double time = -1;

    bool optimal() const { return solution.feasible() && solution.weight() == upper_bound; }
    Weight lower_bound() const { return solution.weight(); }
    double gap() const;
    void print(optimizationtools::Info& info, const std::stringstream& s) const;

    void update_solution(
            const Solution& solution_new,
            const std::stringstream& s,
            optimizationtools::Info& info) { update_solution(solution_new, -1, s, info); }

    void update_solution(
            const Solution& solution_new,
            ComponentId c,
            const std::stringstream& s,
            optimizationtools::Info& info);

    void update_upper_bound(
            Weight upper_bound_new,
            const std::stringstream& s,
            optimizationtools::Info& info);

    Output& algorithm_end(
            optimizationtools::Info& info);
};

Weight algorithm_end(
        Weight upper_bound,
        optimizationtools::Info& info);

}

