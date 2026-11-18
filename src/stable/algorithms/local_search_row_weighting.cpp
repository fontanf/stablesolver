#include "stablesolver/stable/algorithms/local_search_row_weighting.hpp"

#include "stablesolver/stable/algorithm_formatter.hpp"
#include "stablesolver/stable/algorithms/greedy.hpp"

#include "optimizationtools/containers/indexed_set.hpp"
#include "optimizationtools/containers/doubly_indexed_map.hpp"
#include "optimizationtools/containers/indexed_4ary_heap.hpp"

using namespace stablesolver::stable;

namespace
{

/**
 * The algorithm is the row weighting local search of setcoveringsolver applied
 * to the minimum vertex cover problem: each vertex is a set, each edge is an
 * element, and the stable set is the complement of the cover. It is written
 * here in terms of stable sets:
 * - a set in the cover is a vertex outside the stable set;
 * - an uncovered element is a conflict, that is, an edge with both ends in
 *   the stable set;
 * - removing a set from the cover is adding a vertex to the stable set, and
 *   adding a set to the cover is removing a vertex from the stable set.
 *
 * The score of a vertex is the sum of the penalties of its edges whose other
 * end is in the stable set. For a vertex outside the stable set, it is the
 * penalty added by adding it to the stable set. For a vertex in the stable
 * set, it is the penalty removed by removing it from the stable set.
 */

using PenaltyValue = int64_t;

using VertexPos = VertexId;

struct LocalSearchRowWeightingComponent
{
    /** Last vertex added to the stable set. */
    VertexId vertex_id_last_added = -1;

    /** Last vertex removed from the stable set. */
    VertexId vertex_id_last_removed = -1;

    /** Number of iterations. */
    Counter iterations = 0;

    /** Iteration at which optimizing this component starts (included). */
    Counter itmode_start = 0;

    /** Iteration at which optimizing this component ends (excluded). */
    Counter itmode_end = 0;

    /** Vertices outside the stable set, sorted by score. */
    optimizationtools::Indexed4aryHeap<std::pair<PenaltyValue, double>> scores_out;

    /** Weight of the vertices of the component in the stable set. */
    Weight weight = 0;

    /** Total penalty of the conflicts of the component. */
    PenaltyValue penalty = 0;
};

struct LocalSearchRowWeightingVertex
{
    /** Iteration at which the vertex was last added or removed. */
    Counter timestamp = -1;

    /** Score. */
    PenaltyValue score = 0;
};

class LocalSearchRowWeighting
{

public:

    LocalSearchRowWeighting(
            const Instance& instance,
            const Solution& initial_solution,
            std::mt19937_64& generator):
        instance_(instance),
        generator_(generator),
        contains_(instance.number_of_vertices(), 0),
        vertices_id_to_component_pos_(instance.number_of_vertices(), -1),
        vertices_(instance.number_of_vertices()),
        penalties_(instance.number_of_edges(), 1),
        components_(instance.number_of_components()),
        component_conflicts_(
                instance.number_of_edges(),
                instance.number_of_components() + 1)
    {
        for (ComponentId component_id = 0;
                component_id < instance.number_of_components();
                ++component_id) {
            const Component& component = instance.component(component_id);
            components_[component_id].scores_out
                = optimizationtools::Indexed4aryHeap<std::pair<PenaltyValue, double>>(
                        component.vertices.size());
            for (VertexPos vertex_pos = 0;
                    vertex_pos < (VertexPos)component.vertices.size();
                    ++vertex_pos) {
                vertices_id_to_component_pos_[component.vertices[vertex_pos]] = vertex_pos;
            }
        }

        // The initial solution is feasible, so the scores only depend on the
        // vertices of the initial solution.
        for (VertexId vertex_id: initial_solution.vertices()) {
            const Vertex& vertex = instance.vertex(vertex_id);
            contains_[vertex_id] = 1;
            components_[vertex.component].weight += vertex.weight;
            for (const VertexEdge& edge: vertex.edges)
                vertices_[edge.vertex_id].score += penalties_[edge.edge_id];
        }
        for (VertexId vertex_id = 0;
                vertex_id < instance.number_of_vertices();
                ++vertex_id) {
            if (!contains_[vertex_id])
                update_score_out(vertex_id);
        }
    }

    inline bool contains(VertexId vertex_id) const { return contains_[vertex_id]; }

    inline LocalSearchRowWeightingComponent& component(ComponentId component_id) { return components_[component_id]; }

    inline LocalSearchRowWeightingVertex& vertex(VertexId vertex_id) { return vertices_[vertex_id]; }

    /** Return 'true' iff the given component has no conflict. */
    inline bool feasible(ComponentId component_id) const
    {
        return component_conflicts_.number_of_elements(component_id) == 0;
    }

    /** Get the vertex at a given position of the heap of a component. */
    inline VertexId score_out_vertex(
            ComponentId component_id,
            VertexPos heap_pos) const
    {
        VertexPos vertex_pos = components_[component_id].scores_out.top(heap_pos).first;
        return instance_.component(component_id).vertices[vertex_pos];
    }

    /** Draw randomly a conflict of a component. */
    inline EdgeId random_conflict(ComponentId component_id)
    {
        std::uniform_int_distribution<EdgeId> d_e(
                0, component_conflicts_.number_of_elements(component_id) - 1);
        return *(component_conflicts_.begin(component_id) + d_e(generator_));
    }

    /** Add a vertex to the stable set. */
    void add(VertexId vertex_id)
    {
        const Vertex& vertex = instance_.vertex(vertex_id);
        LocalSearchRowWeightingComponent& component = components_[vertex.component];
        contains_[vertex_id] = 1;
        component.weight += vertex.weight;
        component.scores_out.update_key(
                vertices_id_to_component_pos_[vertex_id],
                {-1, -1});
        component.scores_out.pop();
        for (const VertexEdge& edge: vertex.edges) {
            vertices_[edge.vertex_id].score += penalties_[edge.edge_id];
            if (contains_[edge.vertex_id]) {
                component_conflicts_.set(edge.edge_id, vertex.component);
                component.penalty += penalties_[edge.edge_id];
            } else {
                update_score_out(edge.vertex_id);
            }
        }
    }

    /** Remove a vertex from the stable set. */
    void remove(VertexId vertex_id)
    {
        const Vertex& vertex = instance_.vertex(vertex_id);
        LocalSearchRowWeightingComponent& component = components_[vertex.component];
        contains_[vertex_id] = 0;
        component.weight -= vertex.weight;
        for (const VertexEdge& edge: vertex.edges) {
            vertices_[edge.vertex_id].score -= penalties_[edge.edge_id];
            if (contains_[edge.vertex_id]) {
                component_conflicts_.unset(edge.edge_id);
                component.penalty -= penalties_[edge.edge_id];
            } else {
                update_score_out(edge.vertex_id);
            }
        }
        update_score_out(vertex_id);
    }

    /** Increase the penalties of the conflicts of a vertex. */
    void increase_penalties(VertexId vertex_id)
    {
        const Vertex& vertex = instance_.vertex(vertex_id);
        LocalSearchRowWeightingComponent& component = components_[vertex.component];
        for (const VertexEdge& edge: vertex.edges) {
            if (!contains_[edge.vertex_id])
                continue;
            penalties_[edge.edge_id]++;
            component.penalty++;
            vertices_[vertex_id].score++;
            vertices_[edge.vertex_id].score++;
        }
    }

private:

    /** Update the key of a vertex outside the stable set. */
    inline void update_score_out(VertexId vertex_id)
    {
        components_[instance_.vertex(vertex_id).component].scores_out.update_key(
                vertices_id_to_component_pos_[vertex_id],
                {vertices_[vertex_id].score, d_score_(generator_)});
    }

    /** Instance. */
    const Instance& instance_;

    /** Generator. */
    std::mt19937_64& generator_;

    /** Distribution used to break ties between vertices of same score. */
    std::uniform_real_distribution<double> d_score_ = std::uniform_real_distribution<double>(0, 1);

    /** For each vertex, 1 iff it is in the stable set. */
    std::vector<int8_t> contains_;

    /** For each vertex, its position in its component. */
    std::vector<VertexPos> vertices_id_to_component_pos_;

    /** Vertices. */
    std::vector<LocalSearchRowWeightingVertex> vertices_;

    /** Penalties of the edges. */
    std::vector<PenaltyValue> penalties_;

    /** Components. */
    std::vector<LocalSearchRowWeightingComponent> components_;

    /** Conflicts of each component. */
    optimizationtools::DoublyIndexedMap component_conflicts_;

};

/**
 * Add the best vertex to the stable set, then remove from the stable set an
 * end of a random conflict.
 */
void explore_add_remove_neighborhood(
        LocalSearchRowWeighting& local_search,
        const Instance& instance,
        ComponentId component_id,
        Counter iteration)
{
    LocalSearchRowWeightingComponent& component = local_search.component(component_id);

    // Find the cheapest vertex to add.
    VertexId vertex_id_1_best = -1;
    PenaltyValue p_best = 0;
    for (VertexPos heap_pos = 0;
            heap_pos < 7 && heap_pos < component.scores_out.size();
            ++heap_pos) {
        VertexId vertex_id_1 = local_search.score_out_vertex(component_id, heap_pos);
        PenaltyValue p = local_search.vertex(vertex_id_1).score;

        // Check tabu.
        if (component.penalty + p > 0
                && vertex_id_1 == component.vertex_id_last_removed) {
            continue;
        }

        // Update best move.
        if (vertex_id_1_best == -1
                || p_best > p
                || (p_best == p
                    && local_search.vertex(vertex_id_1_best).timestamp
                    > local_search.vertex(vertex_id_1).timestamp)) {
            vertex_id_1_best = vertex_id_1;
            p_best = p;
        }
    }
    if (vertex_id_1_best == -1)
        vertex_id_1_best = local_search.score_out_vertex(component_id, 0);

    // Apply move.
    local_search.add(vertex_id_1_best);
    local_search.vertex(vertex_id_1_best).timestamp = iteration;
    component.vertex_id_last_added = vertex_id_1_best;
    local_search.increase_penalties(vertex_id_1_best);

    // Draw randomly a conflict.
    // The component was infeasible before adding a vertex to the stable set,
    // so it still is.
    EdgeId edge_id = local_search.random_conflict(component_id);
    const Edge& edge = instance.edge(edge_id);

    // Find the best vertex to remove.
    VertexId vertex_id_2_best = -1;
    p_best = 0;
    for (VertexId vertex_id_2: {edge.vertex_id_1, edge.vertex_id_2}) {
        if (vertex_id_2 == component.vertex_id_last_added)
            continue;
        PenaltyValue p = -local_search.vertex(vertex_id_2).score;

        // Update best move.
        if (vertex_id_2_best == -1
                || p_best > p
                || (p_best == p
                    && local_search.vertex(vertex_id_2_best).timestamp
                    > local_search.vertex(vertex_id_2).timestamp)) {
            vertex_id_2_best = vertex_id_2;
            p_best = p;
        }
    }

    // Apply move.
    local_search.remove(vertex_id_2_best);
    local_search.vertex(vertex_id_2_best).timestamp = iteration;
}

/**
 * Swap an end of a random conflict with a vertex outside the stable set.
 */
void explore_swap_neighborhood(
        LocalSearchRowWeighting& local_search,
        const Instance& instance,
        ComponentId component_id,
        Counter iteration)
{
    LocalSearchRowWeightingComponent& component = local_search.component(component_id);
    PenaltyValue penalty = component.penalty;

    // Draw randomly a conflict.
    EdgeId edge_id = local_search.random_conflict(component_id);
    const Edge& edge = instance.edge(edge_id);

    // Find the best swap move.
    VertexId vertex_id_1_best = -1;
    VertexId vertex_id_2_best = -1;
    PenaltyValue p_best = 0;
    // For each end of the conflict which is not the last vertex added to the
    // stable set.
    for (VertexId vertex_id_1: {edge.vertex_id_1, edge.vertex_id_2}) {
        if (vertex_id_1 == component.vertex_id_last_added)
            continue;
        PenaltyValue p0 = -local_search.vertex(vertex_id_1).score;
        if (vertex_id_1_best != -1 && p0 > p_best)
            continue;

        local_search.remove(vertex_id_1);

        for (VertexPos heap_pos = 0;
                heap_pos < 7 && heap_pos < component.scores_out.size();
                ++heap_pos) {
            VertexId vertex_id_2 = local_search.score_out_vertex(component_id, heap_pos);
            if (vertex_id_2 == vertex_id_1)
                continue;
            PenaltyValue p = p0 + local_search.vertex(vertex_id_2).score;

            // Check tabu.
            if (penalty + p > 0
                    && vertex_id_2 == component.vertex_id_last_removed) {
                continue;
            }

            // Update best move.
            if (vertex_id_1_best == -1
                    || p_best > p
                    || (p_best == p
                        && local_search.vertex(vertex_id_1_best).timestamp
                        + local_search.vertex(vertex_id_2_best).timestamp
                        > local_search.vertex(vertex_id_1).timestamp
                        + local_search.vertex(vertex_id_2).timestamp)) {
                vertex_id_1_best = vertex_id_1;
                vertex_id_2_best = vertex_id_2;
                p_best = p;
            }
        }

        local_search.add(vertex_id_1);
    }

    // Apply best swap.
    if (vertex_id_1_best != -1) {
        local_search.remove(vertex_id_1_best);
        local_search.add(vertex_id_2_best);
        if (component.penalty != penalty + p_best) {
            throw std::logic_error(
                    "stablesolver::stable::local_search_row_weighting: "
                    "wrong penalty; "
                    "vertex_id_1_best: " + std::to_string(vertex_id_1_best) + "; "
                    "vertex_id_2_best: " + std::to_string(vertex_id_2_best) + "; "
                    "penalty: " + std::to_string(penalty) + "; "
                    "p_best: " + std::to_string(p_best) + "; "
                    "component.penalty: " + std::to_string(component.penalty) + ".");
        }
        local_search.vertex(vertex_id_1_best).timestamp = iteration;
        local_search.vertex(vertex_id_2_best).timestamp = iteration;
        local_search.increase_penalties(vertex_id_2_best);
    }

    // Update tabu.
    component.vertex_id_last_removed = vertex_id_1_best;
    component.vertex_id_last_added = vertex_id_2_best;
}

}

const LocalSearchRowWeightingOutput stablesolver::stable::local_search_row_weighting(
        const Instance& instance,
        std::mt19937_64& generator,
        const LocalSearchRowWeightingParameters& parameters)
{
    LocalSearchRowWeightingOutput output(instance);
    AlgorithmFormatter algorithm_formatter(parameters, output);
    algorithm_formatter.start("Row weighting local search");

    // Reduction.
    if (parameters.reduction_parameters.reduce) {
        return solve_reduced_instance(
                [&generator](
                    const Instance& instance,
                    const LocalSearchRowWeightingParameters& parameters)
                {
                    return local_search_row_weighting(
                            instance,
                            generator,
                            parameters);
                },
                instance,
                parameters,
                algorithm_formatter,
                output);
    }

    algorithm_formatter.print_header();

    // Compute initial greedy solution.
    // The local search requires a feasible initial solution.
    GreedyParameters greedy_parameters;
    greedy_parameters.verbosity_level = 0;
    greedy_parameters.timer = parameters.timer;
    greedy_parameters.reduction_parameters.reduce = false;
    Solution solution = greedy_gwmin(instance, greedy_parameters).solution;
    algorithm_formatter.update_solution(solution, "initial solution");

    if (instance.number_of_edges() == 0) {
        algorithm_formatter.end();
        return output;
    }

    // Initialize local search structures.
    LocalSearchRowWeighting local_search(instance, solution, generator);

    // The components without edges are never optimized. The iterations are
    // shared between the other ones proportionally to their number of edges.
    for (ComponentId component_id = 0;
            component_id < instance.number_of_components();
            ++component_id) {
        LocalSearchRowWeightingComponent& component = local_search.component(component_id);
        component.itmode_start = (component_id == 0)? 0:
            local_search.component(component_id - 1).itmode_end;
        component.itmode_end = component.itmode_start
            + instance.component(component_id).edges.size();
    }
    Counter itmode_end = local_search.component(instance.number_of_components() - 1).itmode_end;

    // Best solution.
    std::vector<int8_t> solution_best_contains(instance.number_of_vertices(), 0);
    for (VertexId vertex_id: solution.vertices())
        solution_best_contains[vertex_id] = 1;
    std::vector<Weight> solution_best_component_weights(instance.number_of_components(), 0);
    for (ComponentId component_id = 0;
            component_id < instance.number_of_components();
            ++component_id) {
        solution_best_component_weights[component_id] = solution.weight(component_id);
    }
    Weight solution_best_weight = solution.weight();

    auto update_best_solution = [
        &instance,
        &output,
        &algorithm_formatter,
        &solution_best_contains,
        &solution_best_weight]()
    {
        if (output.solution.weight() >= solution_best_weight)
            return;
        Solution solution_best(instance);
        for (VertexId vertex_id = 0;
                vertex_id < instance.number_of_vertices();
                ++vertex_id) {
            if (solution_best_contains[vertex_id])
                solution_best.add(vertex_id);
        }
        std::stringstream ss;
        ss << "it " << output.number_of_iterations;
        algorithm_formatter.update_solution(solution_best, ss.str());
    };

    ComponentId component_id = 0;
    Counter number_of_iterations_without_improvement = 0;
    for (output.number_of_iterations = 0;
            !parameters.timer.needs_to_end();
            ++output.number_of_iterations,
            ++number_of_iterations_without_improvement) {
        // Check stop criteria.
        if (parameters.maximum_number_of_iterations != -1
                && output.number_of_iterations >= parameters.maximum_number_of_iterations)
            break;
        if (parameters.maximum_number_of_iterations_without_improvement != -1
                && number_of_iterations_without_improvement >= parameters.maximum_number_of_iterations_without_improvement)
            break;
        if (solution_best_weight >= output.bound)
            break;

        // Update best solution.
        if (output.number_of_iterations % parameters.best_solution_update_frequency == 0)
            update_best_solution();

        // Compute component.
        Counter itmod = output.number_of_iterations % itmode_end;
        while (itmod < local_search.component(component_id).itmode_start
                || itmod >= local_search.component(component_id).itmode_end) {
            component_id = (component_id + 1) % instance.number_of_components();
        }
        LocalSearchRowWeightingComponent& component = local_search.component(component_id);

        while (local_search.feasible(component_id)) {
            // Update best solution.
            if (solution_best_component_weights[component_id] < component.weight) {
                for (VertexId vertex_id: instance.component(component_id).vertices)
                    solution_best_contains[vertex_id] = local_search.contains(vertex_id);
                solution_best_weight += component.weight
                    - solution_best_component_weights[component_id];
                solution_best_component_weights[component_id] = component.weight;
                number_of_iterations_without_improvement = 0;
            }

            // Add the vertex with the smallest score to the stable set.
            // The component has edges and is feasible, so some of its
            // vertices are outside the stable set.
            VertexId vertex_id = local_search.score_out_vertex(component_id, 0);
            local_search.add(vertex_id);
            local_search.vertex(vertex_id).timestamp = output.number_of_iterations;
            component.vertex_id_last_added = vertex_id;
            local_search.increase_penalties(vertex_id);
        }

        if (component.iterations < 10 * (Counter)instance.component(component_id).vertices.size()) {
            explore_add_remove_neighborhood(
                    local_search,
                    instance,
                    component_id,
                    output.number_of_iterations);
        } else {
            explore_swap_neighborhood(
                    local_search,
                    instance,
                    component_id,
                    output.number_of_iterations);
        }

        component.iterations++;
    }

    update_best_solution();

    algorithm_formatter.end();
    return output;
}
