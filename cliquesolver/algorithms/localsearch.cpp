#include "cliquesolver/algorithms/localsearch.hpp"

#include "cliquesolver/algorithms/greedy.hpp"

#include "optimizationtools/containers/indexed_set.hpp"

#include "localsearchsolver/best_first_local_search.hpp"

using namespace cliquesolver;
using namespace localsearchsolver;

namespace cliquesolver
{

class LocalScheme
{

public:

    /** Global cost: <Weight>; */
    using GlobalCost = std::tuple<Weight>;

    inline Weight&       weight(GlobalCost& global_cost) { return std::get<0>(global_cost); }
    inline Weight  weight(const GlobalCost& global_cost) { return std::get<0>(global_cost); }

    /*
     * Solutions.
     */

    using CompactSolution = std::vector<bool>;

    struct CompactSolutionHasher
    {
        std::hash<CompactSolution> hasher;

        inline bool operator()(
                const std::shared_ptr<CompactSolution>& compact_solution_1,
                const std::shared_ptr<CompactSolution>& compact_solution_2) const
        {
            return *compact_solution_1 == *compact_solution_2;
        }

        inline std::size_t operator()(
                const std::shared_ptr<CompactSolution>& compact_solution) const
        {
            return hasher(*compact_solution);
        }
    };

    inline CompactSolutionHasher compact_solution_hasher() const { return CompactSolutionHasher(); }

    struct Solution
    {
        Solution(VertexId number_of_vertices):
            vertices(number_of_vertices),
            addition_costs(number_of_vertices, 0) {  }

        optimizationtools::IndexedSet vertices;
        std::vector<Weight> addition_costs;
        Weight weight = 0;
    };

    CompactSolution solution2compact(const Solution& solution)
    {
        std::vector<bool> vertices(instance_.graph()->number_of_vertices(), false);
        for (VertexId v: solution.vertices)
            vertices[v] = true;
        return vertices;
    }

    Solution compact2solution(const CompactSolution& compact_solution)
    {
        auto solution = empty_solution();
        for (VertexId v = 0; v < instance_.graph()->number_of_vertices(); ++v)
            if (relevant_vertices_.contains(v))
                if (compact_solution[v])
                    add(solution, v);
        return solution;
    }

    /*
     * Constructors and destructor.
     */

    struct Parameters
    {
        /** Enable (2-1)-swap neighborhood. */
        bool swap_2_1 = true;
        bool shuffle_neighborhood_order = true;
    };

    LocalScheme(
            const Instance& instance,
            Parameters parameters,
            Output& output):
        instance_(instance),
        parameters_(parameters),
        output_(output),
        relevant_vertices_(instance.graph()->number_of_vertices()),
        tight_vertices_(instance.graph()->number_of_vertices(), -1),
        neighbors_(instance_.graph()->number_of_vertices()),
        neighbors_2_(instance_.graph()->number_of_vertices())
    {
        // Initialize relevant_vertices_.
        relevant_vertices_.fill();
    }

    /** Copy constructor. */
    LocalScheme(const LocalScheme& local_scheme):
        LocalScheme(
                local_scheme.instance_,
                local_scheme.parameters_,
                local_scheme.output_) { }

    virtual ~LocalScheme() { }

    /*
     * Initial solutions.
     */

    inline Solution empty_solution() const
    {
        return Solution(instance_.graph()->number_of_vertices());
    }

    inline Solution initial_solution(
            Counter,
            std::mt19937_64&)
    {
        auto output_greedy = greedy_gwmin(instance_);
        Solution solution = empty_solution();
        for (VertexId v: relevant_vertices_)
            if (output_greedy.solution.contains(v))
                add(solution, v);
        return solution;
    }

    /*
     * Solution properties.
     */

    inline GlobalCost global_cost(const Solution& solution) const
    {
        return {
            -solution.weight,
        };
    }

    /*
     * Local search.
     */

    struct Move
    {
        Move(): v(-1) { }

        VertexId v;
        GlobalCost global_cost;
    };

    struct MoveHasher
    {
        std::hash<VertexId> hasher;

        inline bool hashable(const Move&) const { return true; }

        inline bool operator()(
                const Move& move_1,
                const Move& move_2) const
        {
            return move_1.v == move_2.v;
        }

        inline std::size_t operator()(
                const Move& move) const
        {
            size_t hash = hasher(move.v);
            return hash;
        }
    };

    inline MoveHasher move_hasher() const { return MoveHasher(); }

    inline std::vector<Move> perturbations(
            const Solution& solution,
            std::mt19937_64&)
    {
        std::vector<Move> moves;
        for (VertexId v: relevant_vertices_) {
            GlobalCost c = (contains(solution, v))?
                cost_remove(solution, v, worst<GlobalCost>()):
                cost_add(solution, v, worst<GlobalCost>());
            Move move;
            move.v = v;
            move.global_cost = c;
            moves.push_back(move);
        }
        return moves;
    }

    inline void apply_move(Solution& solution, const Move& move) const
    {
        if (contains(solution, move.v)) {
            remove(solution, move.v);
        } else {
            if (relevant_vertices_.contains(move.v))
                add(solution, move.v);
        }
    }

    inline void local_search(
            Solution& solution,
            std::mt19937_64& generator,
            const Move& tabu = Move())
    {
        // Get neighborhoods.
        std::vector<Counter> neighborhoods = {0};
        if (parameters_.swap_2_1)
            neighborhoods.push_back(1);

        // Update core.
        if (best_weight_ < solution.weight) {
            best_weight_ = solution.weight;
            instance_.update_core(relevant_vertices_, solution.weight);
            //std::cout << "weight " << output_.solution.weight()
            //    << " core " << relevant_vertices_.size()
            //    << " / " << instance_.graph()->number_of_vertices()
            //    << std::endl;
            for (VertexId v: solution.vertices) {
                if (!relevant_vertices_.contains(v)) {
                    //std::cout << "remove " << v << std::endl;
                    remove(solution, v);
                }
            }
        }

        Counter it = 0;
        for (;; ++it) {
            //std::cout << "it " << it
            //    << " c " << to_string(global_cost(solution))
            //    << std::endl;
            //print(std::cout, solution);

            if (parameters_.shuffle_neighborhood_order)
                std::shuffle(neighborhoods.begin(), neighborhoods.end(), generator);

            bool improved = false;
            // Loop through neighborhoods.
            for (Counter neighborhood: neighborhoods) {
                switch (neighborhood) {
                case 0: { // Add neighborhood.
                    relevant_vertices_.shuffle_in(generator);
                    VertexId v_best = -1;
                    GlobalCost c_best = global_cost(solution);
                    for (VertexId v: relevant_vertices_) {
                        if (v == tabu.v)
                            continue;
                        if (contains(solution, v))
                            continue;
                        GlobalCost c = cost_add(solution, v, c_best);
                        if (c >= c_best)
                            continue;
                        if (v_best != -1 && !dominates(c, c_best))
                            continue;
                        v_best = v;
                        c_best = c;
                    }
                    //std::cout << "v_best " << v_best << std::endl;
                    if (v_best != -1) {
                        improved = true;
                        // Apply move.
                        assert(!contains(solution, v_best));
                        add(solution, v_best);
                        if (global_cost(solution) != c_best) {
                            throw std::logic_error(
                                    "Add. Costs do not match:\n"
                                    "* v_best: " + std::to_string(v_best) + "\n"
                                    + "* Expected new cost: " + to_string(c_best) + "\n"
                                    + "* Actual new cost: " + to_string(global_cost(solution)) + "\n");
                        }
                    }
                    break;
                } case 1: { // (2-1)-swap neighborhood.
                    // Find vertices connected to all but one vertex of the
                    // current solution.
                    tight_vertices_.clear();
                    relevant_vertices_.shuffle_in(generator);
                    for (VertexId v: relevant_vertices_) {
                        if (v == tabu.v)
                            continue;
                        if (contains(solution, v))
                            continue;
                        VertexId v2 = -1;
                        for (auto it = instance_.graph()->neighbors_begin(v);
                                it != instance_.graph()->neighbors_end(v); ++it) {
                            if (!contains(solution, *it)) {
                                if (v2 != -1) {
                                    v2 = -1;
                                    break;
                                }
                                v2 = *it;
                            }
                        }
                        if (v2 != -1) {
                            tight_vertices_.set(v, v2);
                        }
                    }

                    VertexId v_in_best = -1;
                    VertexId v_out_1_best = -1;
                    VertexId v_out_2_best = -1;
                    GlobalCost c_best = global_cost(solution);
                    for (const auto& p: tight_vertices_) {
                        VertexId v_out_1 = p.first;
                        VertexId v_in = p.second;
                        for (auto it = instance_.graph()->neighbors_begin(v_out_1);
                                it != instance_.graph()->neighbors_end(v_out_1); ++it) {
                            VertexId v_out_2 = *it;
                            if (tight_vertices_.contains(v_out_2)
                                    && tight_vertices_[v_out_2] == v_in) {
                                GlobalCost c = -(solution.weight
                                        + instance_.graph()->weight(v_out_1)
                                        + instance_.graph()->weight(v_out_2)
                                        - instance_.graph()->weight(v_in));
                                if (c >= c_best)
                                    continue;
                                if (v_in_best != -1 && !dominates(c, c_best))
                                    continue;
                                v_in_best = v_in;
                                v_out_1_best = v_out_1;
                                v_out_2_best = v_out_2;
                                c_best = c;
                            }
                        }
                    }
                    if (v_in_best != -1) {
                        improved = true;
                        // Apply move.
                        assert(contains(solution, v_in_best));
                        remove(solution, v_in_best);
                        assert(!contains(solution, v_out_1_best));
                        add(solution, v_out_1_best);
                        assert(!contains(solution, v_out_2_best));
                        add(solution, v_out_2_best);
                        if (global_cost(solution) != c_best) {
                            throw std::logic_error(
                                    "(2,1)-swap. Costs do not match:\n"
                                    "* Expected new cost: " + to_string(c_best) + "\n"
                                    + "* Actual new cost: " + to_string(global_cost(solution)) + "\n");
                        }
                    }
                    break;
                }
                }
                if (improved)
                    break;
            }
            if (!improved)
                break;
        }
        //print(std::cout, solution);

        // Update core.
        if (best_weight_ < solution.weight) {
            best_weight_ = solution.weight;
            instance_.update_core(relevant_vertices_, solution.weight);
        }
    }

    /*
     * Outputs.
     */

    std::ostream& print(
            std::ostream &os,
            const Solution& solution)
    {
        //os << "vertices" << std::endl;
        //for (VertexId v: solution.vertices) {
        //    os << v << ":" << std::endl;
        //    for (auto it = instance_.graph()->neighbors_begin(v);
        //            it != instance_.graph()->neighbors_end(v); ++it) {
        //        os << " " << *it;
        //    }
        //    std::cout << std::endl;
        //}

        os << "vertices:";
        for (VertexId v = 0; v < instance_.graph()->number_of_vertices(); ++v)
            if (contains(solution, v))
                os << " " << v;
        os << std::endl;
        os << "weight: " << solution.weight << std::endl;
        return os;
    }

    inline void write(const Solution&, std::string) const { return; }

private:

    /*
     * Manipulate solutions.
     */

    inline bool contains(const Solution& solution, VertexId v) const
    {
        return solution.vertices.contains(v);
    }

    inline void add(Solution& solution, VertexId v) const
    {
        assert(v >= 0);
        assert(!contains(solution, v));
        assert(relevant_vertices_.contains(v));
        //std::cout << "add " << v
        //    << " c " << solution.addition_costs[v]
        //    << " rv " << relevant_vertices_.contains(v)
        //    << std::endl;

        neighbors_.clear();
        neighbors_.add(v);
        for (auto it = instance_.graph()->neighbors_begin(v);
                it != instance_.graph()->neighbors_end(v); ++it) {
            neighbors_.add(*it);
        }

        Weight w = instance_.graph()->weight(v);

        // Remove conflicting vertices.
        for (VertexId v2: relevant_vertices_) {
            if (!neighbors_.contains(v2)) {
                if (contains(solution, v2))
                    remove(solution, v2);
                solution.addition_costs[v2] += w;
            }
        }

        solution.vertices.add(v);
        solution.weight += w;
        assert(solution.addition_costs[v] == 0);
    }

    inline void remove(Solution& solution, VertexId v) const
    {
        assert(v >= 0);
        assert(contains(solution, v));
        //std::cout << "remove " << v
        //    << " rv " << relevant_vertices_.contains(v)
        //    << std::endl;

        neighbors_2_.clear();
        neighbors_2_.add(v);
        for (auto it = instance_.graph()->neighbors_begin(v);
                it != instance_.graph()->neighbors_end(v); ++it) {
            neighbors_2_.add(*it);
        }

        Weight w = instance_.graph()->weight(v);

        solution.vertices.remove(v);
        solution.weight -= w;
        for (VertexId v2: relevant_vertices_) {
            if (!neighbors_2_.contains(v2)) {
                solution.addition_costs[v2] -= w;
                assert(solution.addition_costs[v2] >= 0);
            }
        }
    }

    /*
     * Evaluate moves.
     */

    inline GlobalCost cost_remove(const Solution& solution, VertexId v, GlobalCost) const
    {
        return {
            - (solution.weight - instance_.graph()->weight(v)),
        };
    }

    inline GlobalCost cost_add(const Solution& solution, VertexId v, GlobalCost) const
    {
        return {
            -(solution.weight
                    + instance_.graph()->weight(v)
                    - solution.addition_costs[v]),
        };
    }

    /*
     * Private attributes.
     */

    const Instance& instance_;
    Parameters parameters_;
    Output& output_;
    Weight best_weight_ = 0;

    optimizationtools::IndexedSet relevant_vertices_;
    optimizationtools::IndexedMap<VertexId> tight_vertices_;
    mutable optimizationtools::IndexedSet neighbors_;
    mutable optimizationtools::IndexedSet neighbors_2_;

};

}

LocalSearchOutput& LocalSearchOutput::algorithm_end(
        optimizationtools::Info& info)
{
    //FFOT_PUT(info, "Algorithm", "Iterations", iterations);
    Output::algorithm_end(info);
    //FFOT_VER(info, "Iterations: " << iterations << std::endl);
    return *this;
}

LocalSearchOutput cliquesolver::localsearch(
        const Instance& instance,
        std::mt19937_64&,
        LocalSearchOptionalParameters parameters)
{
    cliquesolver::init_display(instance, parameters.info);
    FFOT_VER(parameters.info,
               "Algorithm" << std::endl
            << "---------" << std::endl
            << "Local Search" << std::endl
            << std::endl);

    LocalSearchOutput output(instance, parameters.info);

    // Create LocalScheme.
    LocalScheme::Parameters parameters_local_scheme;
    LocalScheme local_scheme(instance, parameters_local_scheme, output);

    // Run A*.
    BestFirstLocalSearchOptionalParameters<LocalScheme> parameters_best_first;
    //parameters_best_first.info.set_verbose(true);
    parameters_best_first.info.set_time_limit(parameters.info.remaining_time());
    parameters_best_first.maximum_number_of_nodes = parameters.maximum_number_of_nodes;
    parameters_best_first.number_of_threads_1 = 1;
    parameters_best_first.number_of_threads_2 = parameters.number_of_threads;
    parameters_best_first.initial_solution_ids = std::vector<Counter>(
            parameters_best_first.number_of_threads_2, 0);
    bool end = false;
    parameters_best_first.info.end = &end;
    parameters_best_first.new_solution_callback
        = [&instance, &parameters, &output, &parameters_best_first](
                const LocalScheme::Solution& solution)
        {
            Solution sol(instance);
            for (VertexId v = 0; v < instance.graph()->number_of_vertices(); ++v)
                if (solution.vertices.contains(v))
                    sol.add(v);
            std::stringstream ss;
            output.update_solution(sol, ss, parameters.info);

            optimizationtools::IndexedSet relevant_vertices(instance.graph()->number_of_vertices());
            relevant_vertices.fill();
            Weight upper_bound = instance.update_core(relevant_vertices, output.solution.weight());
            output.update_upper_bound(upper_bound, ss, parameters.info);

            if (output.optimal())
                *(parameters_best_first.info.end) = true;
        };
    best_first_local_search(local_scheme, parameters_best_first);

    return output.algorithm_end(parameters.info);
}

