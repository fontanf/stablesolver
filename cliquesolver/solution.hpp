#pragma once

#include "cliquesolver/instance.hpp"

#include "optimizationtools/containers/indexed_set.hpp"
#include "optimizationtools/containers/indexed_map.hpp"

#include <functional>

namespace cliquesolver
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

    /*
     * Getters.
     */

    /** Get the instance. */
    inline const Instance& instance() const { return *instance_; }

    /** Get the number of vertices of the solution. */
    inline VertexId number_of_vertices() const { return vertices_.size(); }

    /** Get the weight of the solution. */
    inline Weight weight() const { return weight_; }

    /** Get the penalty of the solution. */
    inline Weight penalty() const { return penalty_; }

    /** Return 'true' iff vertex v is in the solution. */
    inline int8_t contains(VertexId e) const { return vertices_.contains(e); }

    /** Return 'true' iff the solution is feasible. */
    inline bool feasible() const { return penalty() == 0; }

    /** Get the set of vertices of the solution. */
    const optimizationtools::IndexedSet& vertices() const { return vertices_; };

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
    const Instance* instance_;

    /** Set of vertices of the solution. */
    optimizationtools::IndexedSet vertices_;

    /** Weight of the solution. */
    Weight weight_ = 0;

    /** Penalty of the solution. */
    Weight penalty_ = 0;

    optimizationtools::IndexedSet neighbors_tmp_;

};

void Solution::add(VertexId v)
{
    instance().graph()->check_vertex_index(v);
    assert(!contains(v));
    neighbors_tmp_.clear();
    for (auto it = instance().graph()->neighbors_begin(v);
            it != instance().graph()->neighbors_end(v); ++it) {
        neighbors_tmp_.add(*it);
    }
    for (VertexId v2: vertices())
        if (!neighbors_tmp_.contains(v2))
            penalty_ += 1;
    vertices_.add(v);
    weight_ += instance().graph()->weight(v);
}

void Solution::remove(VertexId v)
{
    instance().graph()->check_vertex_index(v);
    assert(contains(v));
    vertices_.remove(v);
    weight_ -= instance().graph()->weight(v);
    neighbors_tmp_.clear();
    for (auto it = instance().graph()->neighbors_begin(v);
            it != instance().graph()->neighbors_end(v); ++it) {
        neighbors_tmp_.add(*it);
    }
    for (VertexId v2: vertices())
        if (!neighbors_tmp_.contains(v2))
            penalty_ -= 1;
}

std::ostream& operator<<(std::ostream& os, const Solution& solution);

////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// Output ////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

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

