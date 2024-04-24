#pragma once

#include "stablesolver/stable/solution.hpp"

namespace stablesolver
{
namespace stable
{

/**
 * Structure passed as parameters of the reduction algorithm and the other
 * algorithm to determine whether and how to reduce.
 */
struct ReductionParameters
{
    /** Boolean indicating if the reduction should be performed. */
    bool reduce = true;

    /** Maximum number of rounds. */
    Counter maximum_number_of_rounds = 10;
};

class Reduction
{

public:

    /** Constructor. */
    Reduction(
            const Instance& instance,
            const ReductionParameters& parameters = {});

    /** Get the reduced instance. */
    const Instance& instance() const { return instance_; };

    /** Unreduce a solution of the reduced instance. */
    Solution unreduce_solution(
            const Solution& solution) const;

    /** Unreduce a bound of the reduced instance. */
    Weight unreduce_bound(
            Weight bound) const;

private:

    /*
     * Private methods
     */

    /**
     * Perform pendant vertices reduction.
     *
     * See:
     * - "Accelerating Local Search for the Maximum Independent Set Problem"
     *   (Dahlum et al., 2016)
     *   https://doi.org/10.1007/978-3-319-38851-9_9
     */
    bool reduce_pendant_vertices();

    /**
     * Perform isolated vertex removal reduction.
     *
     * See:
     * - "Exactly Solving the Maximum Weight Independent Set Problem on Large
     *   Real-World Graphs" (Lamm et al., 2019)
     *   https://doi.org/10.1137/1.9781611975499.12
     */
    bool reduce_isolated_vertex_removal();

    /**
     * Perform vertex folding reduction.
     *
     * See:
     * - "Branch-and-reduce exponential/FPT algorithms in practice: A case
     *   study of vertex cover" (Akibaa et Iwata, 2016)
     *   https://doi.org/10.1016/j.tcs.2015.09.023
     * - "Exactly Solving the Maximum Weight Independent Set Problem on Large
     *   Real-World Graphs" (Lamm et al., 2019)
     *   https://doi.org/10.1137/1.9781611975499.12
     */
    bool reduce_vertex_folding();

    /**
     * Perform twin reduction.
     *
     * See:
     * - "Branch-and-reduce exponential/FPT algorithms in practice: A case
     *   study of vertex cover" (Akibaa et Iwata, 2016)
     *   https://doi.org/10.1016/j.tcs.2015.09.023
     * - "Exactly Solving the Maximum Weight Independent Set Problem on Large
     *   Real-World Graphs" (Lamm et al., 2019)
     *   https://doi.org/10.1137/1.9781611975499.12
     */
    bool reduce_twin();

    /**
     * Perform domination reduction.
     *
     * See:
     * - "Branch-and-reduce exponential/FPT algorithms in practice: A case
     *   study of vertex cover" (Akibaa et Iwata, 2016)
     *   https://doi.org/10.1016/j.tcs.2015.09.023
     * - "Exactly Solving the Maximum Weight Independent Set Problem on Large
     *   Real-World Graphs" (Lamm et al., 2019)
     *   https://doi.org/10.1137/1.9781611975499.12
     */
    bool reduce_domination();

    /**
     * Perform unconfined reduction.
     *
     * The unconfined reduction rule is a generalization of the dominance and
     * the satellite reduction rules.
     *
     * See:
     * - "Confining sets and avoiding bottleneck cases: A simple maximum
     *   independent set algorithm in degree-3 graphs" (Xiao et
     *   HiroshiNagamochi, 2013)
     *   https://doi.org/10.1016/j.tcs.2012.09.022
     * - "Accelerating Local Search for the Maximum Independent Set Problem"
     *   (Dahlum et al., 2016)
     *   https://doi.org/10.1007/978-3-319-38851-9_9
     * - "Exactly Solving the Maximum Weight Independent Set Problem on Large
     *   Real-World Graphs" (Lamm et al., 2019)
     *   https://doi.org/10.1137/1.9781611975499.12
     */
    bool reduce_unconfined();

    /*
     * Private attributes
     */

    /** Original instance. */
    const Instance* original_instance_ = nullptr;

    /** Reduced instance. */
    Instance instance_;

    /**
     * Structure that stores the unreduction operation for a considered vertex.
     */
    struct UnreductionOperations
    {
        /**
         * List of vertices from the original graph to add if the considered vertex
         * is in the solution of the reduced instance.
         */
        std::vector<VertexId> in;

        /**
         * List of vertices from the original graph to add if the considered vertex
         * is NOT in the solution of the reduced instance.
         */
        std::vector<VertexId> out;
    };

    /** For each vertex, unreduction operations. */
    std::vector<UnreductionOperations> unreduction_operations_;

    /** Mandatory vertices (from the original instance). */
    std::vector<VertexId> mandatory_vertices_;

    /**
     * Weight to add to a solution of the reduced instance to get the weight of
     * the corresponding solution of the original instance.
     */
    Weight extra_weight_;

};

}
}
