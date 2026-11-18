#pragma once

#include "stablesolver/stable/algorithm.hpp"

namespace stablesolver
{
namespace stable
{

struct LocalSearchRowWeightingParameters: Parameters
{
    /** Maximum number of iterations. */
    Counter maximum_number_of_iterations = -1;

    /** Maximum number of iterations without improvement. */
    Counter maximum_number_of_iterations_without_improvement = -1;

    /**
     * Best solution update frequency.
     *
     * Updating the best solution rebuilds the whole solution. On graphs with
     * many components, the best solution improves at almost every iteration,
     * so updating it at every iteration slows the algorithm down a lot.
     */
    Counter best_solution_update_frequency = 100000;

    virtual int format_width() const override { return 37; }

    virtual void format(std::ostream& os) const override
    {
        Parameters::format(os);
        int width = format_width();
        os
            << std::setw(width) << std::left << "Max. # of iterations: " << maximum_number_of_iterations << std::endl
            << std::setw(width) << std::left << "Max. # of iterations without impr.: " << maximum_number_of_iterations_without_improvement << std::endl
            << std::setw(width) << std::left << "Best solution update frequency: " << best_solution_update_frequency << std::endl
            ;
    }

    virtual nlohmann::json to_json() const override
    {
        nlohmann::json json = Parameters::to_json();
        json.merge_patch({
                {"MaximumNumberOfIterations", maximum_number_of_iterations},
                {"MaximumNumberOfIterationsWithoutImprovement", maximum_number_of_iterations_without_improvement},
                {"BestSolutionUpdateFrequency", best_solution_update_frequency},
                });
        return json;
    }
};

struct LocalSearchRowWeightingOutput: Output
{
    LocalSearchRowWeightingOutput(
            const Instance& instance):
        Output(instance) { }


    /** Number of iterations. */
    Counter number_of_iterations = 0;


    virtual int format_width() const override { return 31; }

    virtual void format(std::ostream& os) const override
    {
        Output::format(os);
        int width = format_width();
        os
            << std::setw(width) << std::left << "Number of iterations: " << number_of_iterations << std::endl
            ;
    }

    virtual nlohmann::json to_json() const override
    {
        nlohmann::json json = Output::to_json();
        json.merge_patch({
                {"NumberOfIterations", number_of_iterations},
                });
        return json;
    }
};

/**
 * Row weighting local search for the unweighted maximum independent set
 * problem.
 *
 * It is the local search of setcoveringsolver applied to the minimum vertex
 * cover problem: each vertex is a set, each edge is an element, and the
 * stable set is the complement of the cover.
 */
const LocalSearchRowWeightingOutput local_search_row_weighting(
        const Instance& instance,
        std::mt19937_64& generator,
        const LocalSearchRowWeightingParameters& parameters = {});

}
}
