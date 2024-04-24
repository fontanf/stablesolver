#pragma once

#include "stablesolver/stable/instance.hpp"

#include "optimizationtools/containers/indexed_set.hpp"

#include "nlohmann/json.hpp"

#include <unordered_set>

namespace stablesolver
{
namespace stable
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

    /** Get the weight of connected component c. */
    inline Weight weight(ComponentId c) const { return component_weights_[c]; }

    /** Return 'true' iff vertex v is in the solution. */
    inline int8_t contains(VertexId vertex_id) const { return vertices_.contains(vertex_id); }

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
    if (contains(instance().edge(e).vertex_id_1)) {
        if (contains(instance().edge(e).vertex_id_2)) {
            return 2;
        } else {
            return 1;
        }
    } else {
        if (contains(instance().edge(e).vertex_id_2)) {
            return 1;
        } else {
            return 0;
        }
    }
}

void Solution::add(VertexId vertex_id)
{
    // Checks.
    instance().check_vertex_index(vertex_id);
    if (contains(vertex_id)) {
        throw std::invalid_argument(
                "Cannot add vertex " + std::to_string(vertex_id)
                + " which is already in the solution.");
    }

    ComponentId c = instance().vertex(vertex_id).component;
    for (const auto& edge: instance().vertex(vertex_id).edges) {
        if (covers(edge.edge_id) == 1) {
            component_number_of_conflictss_[c]++;
            conflicts_.insert(edge.edge_id);
        }
    }
    weight_ += instance().vertex(vertex_id).weight;
    component_weights_[c] += instance().vertex(vertex_id).weight;
    vertices_.add(vertex_id);
}

void Solution::remove(VertexId vertex_id)
{
    // Checks.
    instance().check_vertex_index(vertex_id);
    if (!contains(vertex_id)) {
        throw std::invalid_argument(
                "Cannot remove vertex " + std::to_string(vertex_id)
                + " which is not in the solution.");
    }

    ComponentId c = instance().vertex(vertex_id).component;
    for (const auto& edge: instance().vertex(vertex_id).edges) {
        if (covers(edge.edge_id) == 2) {
            component_number_of_conflictss_[c]--;
            conflicts_.erase(edge.edge_id);
        }
    }
    weight_ -= instance().vertex(vertex_id).weight;
    component_weights_[c] -= instance().vertex(vertex_id).weight;
    vertices_.remove(vertex_id);
}

}
}
