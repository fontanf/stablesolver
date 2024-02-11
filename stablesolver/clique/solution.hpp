#pragma once

#include "stablesolver/clique/instance.hpp"

#include "optimizationtools/containers/indexed_set.hpp"
#include "optimizationtools/utils/output.hpp"
#include "optimizationtools/utils/utils.hpp"

#include <cassert>
#include <iomanip>

namespace stablesolver
{
namespace clique
{

class Solution
{

public:

    /*
     * Constructors and destructor
     */

    /** Create an empty solution. */
    Solution(const Instance& instance);

    /** Create a solution from a certificate file. */
    Solution(
            const Instance& instance,
            const std::string& certificate_path);

    /*
     * Getters
     */

    /** Get the instance. */
    inline const Instance& instance() const { return *instance_; }

    /** Get the number of vertices of the solution. */
    inline VertexId number_of_vertices() const { return vertices_.size(); }

    /** Get the weight of the solution. */
    inline Weight weight() const { return weight_; }

    /** Get the objective value of the solution. */
    inline Weight objective_value() const { return weight(); }

    /** Get the penalty of the solution. */
    inline Weight penalty() const { return penalty_; }

    /** Return 'true' iff vertex v is in the solution. */
    inline int8_t contains(VertexId e) const { return vertices_.contains(e); }

    /** Return 'true' iff the solution is feasible. */
    inline bool feasible() const { return penalty() == 0; }

    /** Get the set of vertices of the solution. */
    const optimizationtools::IndexedSet& vertices() const { return vertices_; };

    /*
     * Setters
     */

    /** Add vertex v to the solution. */
    inline void add(VertexId v);

    /** Remove vertex v from the solution. */
    inline void remove(VertexId v);

    /*
     * Export
     */

    /** Print the instance. */
    std::ostream& format(
            std::ostream& os,
            int verbosity_level = 1) const;

    /** Write the solution to a file. */
    void write(const std::string& certificate_path) const;

    /** Export solution characteristics to a JSON structure. */
    nlohmann::json to_json() const;

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

inline optimizationtools::ObjectiveDirection objective_direction()
{
    return optimizationtools::ObjectiveDirection::Maximize;
}

/**
 * Output structure for a maximum-weight clique problem.
 */
struct Output: optimizationtools::Output
{
    /** Constructor. */
    Output(const Instance& instance):
        solution(instance),
        bound(instance.graph()->total_weight()) { }


    /** Solution. */
    Solution solution;

    /** Bound. */
    Weight bound = 0;

    /** Elapsed time. */
    double time = 0.0;


    std::string solution_value() const
    {
        return optimizationtools::solution_value(
            objective_direction(),
            solution.feasible(),
            solution.objective_value());
    }

    double absolute_optimality_gap() const
    {
        return optimizationtools::absolute_optimality_gap(
                objective_direction(),
                solution.feasible(),
                solution.objective_value(),
                bound);
    }

    double relative_optimality_gap() const
    {
       return optimizationtools::relative_optimality_gap(
            objective_direction(),
            solution.feasible(),
            solution.objective_value(),
            bound);
    }

    bool optimal() const { return absolute_optimality_gap() == 0.0; }

    virtual nlohmann::json to_json() const
    {
        return nlohmann::json {
            {"Solution", solution.to_json()},
            {"Value", solution_value()},
            {"Bound", bound},
            {"AbsoluteOptimalityGap", absolute_optimality_gap()},
            {"RelativeOptimalityGap", relative_optimality_gap()},
            {"Time", time}
        };
    }

    virtual int format_width() const { return 30; }

    virtual void format(std::ostream& os) const
    {
        int width = format_width();
        os
            << std::setw(width) << std::left << "Value: " << solution_value() << std::endl
            << std::setw(width) << std::left << "Bound: " << bound << std::endl
            << std::setw(width) << std::left << "Absolute optimality gap: " << absolute_optimality_gap() << std::endl
            << std::setw(width) << std::left << "Relative optimality gap (%): " << relative_optimality_gap() * 100 << std::endl
            << std::setw(width) << std::left << "Time (s): " << time << std::endl
            ;
    }
};

using NewSolutionCallback = std::function<void(const Output&, const std::string&)>;

struct Parameters: optimizationtools::Parameters
{
    /** Callback function called when a new best solution is found. */
    NewSolutionCallback new_solution_callback = [](const Output&, const std::string&) { };


    virtual nlohmann::json to_json() const override
    {
        nlohmann::json json = optimizationtools::Parameters::to_json();
        json.merge_patch({});
        return json;
    }

    virtual int format_width() const override { return 23; }

    virtual void format(std::ostream& os) const override
    {
        optimizationtools::Parameters::format(os);
        //int width = format_width();
        //os
        //    ;
    }
};

}
}
