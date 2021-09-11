#include "stablesolver/algorithms/localsearch.hpp"

#include "localsearchsolver/a_star_local_search.hpp"

using namespace stablesolver;
using namespace localsearchsolver;

namespace stablesolver
{

class LocalScheme
{

public:

    /** Global cost: <Overweight, Weight>; */
    using GlobalCost = std::tuple<Weight>;

    inline Weight&       weight(GlobalCost& global_cost) { return std::get<0>(global_cost); }
    inline Weight  weight(const GlobalCost& global_cost) { return std::get<0>(global_cost); }

    static GlobalCost global_cost_worst()
    {
        return {
            std::numeric_limits<Weight>::max(),
        };
    }

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

    struct SolutionVertex
    {
        /**
         * in == true iff vertex j is in the solution.
         */
        bool in = false;

        /**
         * neighbor_weight = p iff the sum of the weights of the neighbors of j
         * which are in the solution is equal to p.
         */
        Weight neighbor_weight = 0;
    };

    struct Solution
    {
        std::vector<SolutionVertex> vertices;
        Weight weight = 0;
    };

    CompactSolution solution2compact(const Solution& solution)
    {
        std::vector<bool> vertices(instance_.number_of_vertices(), false);
        for (VertexId v = 0; v < instance_.number_of_vertices(); ++v)
            if (solution.vertices[v].in)
                vertices[v] = true;
        return vertices;
    }

    Solution compact2solution(const CompactSolution& compact_solution)
    {
        auto solution = empty_solution();
        for (VertexId v = 0; v < instance_.number_of_vertices(); ++v)
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
            Parameters parameters):
        instance_(instance),
        parameters_(parameters),
        vertices_(instance.number_of_vertices()),
        neighbors_(instance_.number_of_vertices()),
        free_vertices_(instance_.number_of_vertices()),
        free_vertices_2_(instance_.number_of_vertices())
    {
        // Initialize vertices_.
        std::iota(vertices_.begin(), vertices_.end(), 0);
    }

    LocalScheme(const LocalScheme& local_scheme):
        LocalScheme(local_scheme.instance_, local_scheme.parameters_) { }

    virtual ~LocalScheme() { }

    /*
     * Initial solutions.
     */

    inline Solution empty_solution() const
    {
        Solution solution;
        solution.vertices.resize(instance_.number_of_vertices());
        return solution;
    }

    inline Solution initial_solution(
            Counter,
            std::mt19937_64& generator)
    {
        Solution solution = empty_solution();
        std::shuffle(vertices_.begin(), vertices_.end(), generator);
        for (VertexId v: vertices_)
            if (solution.vertices[v].neighbor_weight == 0)
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
        VertexId v;
        GlobalCost global_cost;
    };

    static Move move_null() { return {-1, global_cost_worst()}; }

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
        for (VertexId v: vertices_) {
            GlobalCost c = (contains(solution, v))?
                cost_remove(solution, v, global_cost_worst()):
                cost_add(solution, v, global_cost_worst());
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
            add(solution, move.v);
        }
    }

    inline void local_search(
            Solution& solution,
            std::mt19937_64& generator,
            const Move& tabu = move_null())
    {
        // Get neighborhoods.
        std::vector<Counter> neighborhoods = {0};
        if (parameters_.swap_2_1)
            neighborhoods.push_back(1);

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
                case 0: { // Toggle neighborhood.
                    std::shuffle(vertices_.begin(), vertices_.end(), generator);
                    VertexId v_best = -1;
                    GlobalCost c_best = global_cost(solution);
                    for (VertexId v: vertices_) {
                        if (v == tabu.v)
                            continue;
                        GlobalCost c = (contains(solution, v))?
                            cost_remove(solution, v, c_best):
                            cost_add(solution, v, c_best);
                        if (c >= c_best)
                            continue;
                        if (v_best != -1 && !dominates(c, c_best))
                            continue;
                        v_best = v;
                        c_best = c;
                    }
                    if (v_best != -1) {
                        improved = true;
                        // Apply move.
                        if (contains(solution, v_best)) {
                            remove(solution, v_best);
                        } else {
                            add(solution, v_best);
                        }
                        assert(global_cost(solution) == c_best);
                    }
                    break;
                } case 1: { // (2-1)-swap neighborhood.
                    std::shuffle(vertices_.begin(), vertices_.end(), generator);
                    // Get vertices inside the knapsack.
                    vertices_in_.clear();
                    for (VertexId v: vertices_) {
                        if (v == tabu.v)
                            continue;
                        if (contains(solution, v))
                            vertices_in_.push_back(v);
                    }

                    VertexId v_in_best = -1;
                    VertexId v_out_1_best = -1;
                    VertexId v_out_2_best = -1;
                    GlobalCost c_best = global_cost(solution);
                    for (VertexId v_in: vertices_in_) {
                        // Update free_vertices_
                        free_vertices_.clear();
                        for (const VertexEdge& edge: instance_.vertex(v_in).edges)
                            if (solution.vertices[edge.v].neighbor_weight
                                    == instance_.vertex(v_in).weight)
                                free_vertices_.add(edge.v);
                        if (free_vertices_.size() <= 2)
                            continue;
                        free_vertices_.shuffle_in(generator);
                        remove(solution, v_in);
                        for (VertexId v_out_1: free_vertices_) {
                            assert(v_out_1 != v_in);
                            free_vertices_2_.clear();
                            for (VertexId v: free_vertices_)
                                free_vertices_2_.add(v);
                            free_vertices_2_.remove(v_out_1);
                            for (const VertexEdge& edge: instance_.vertex(v_out_1).edges)
                                if (free_vertices_2_.contains(edge.v))
                                    free_vertices_2_.remove(edge.v);
                            if (free_vertices_2_.empty())
                                continue;
                            free_vertices_2_.shuffle_in(generator);
                            assert(!contains(solution, v_out_1));
                            add(solution, v_out_1);
                            for (VertexId v_out_2: free_vertices_2_) {
                                assert(v_out_2 != v_in);
                                assert(v_out_2 != v_out_1);
                                GlobalCost c = cost_add(solution, v_out_2, c_best);
                                if (c >= c_best)
                                    continue;
                                if (v_in_best != -1 && !dominates(c, c_best))
                                    continue;
                                v_in_best = v_in;
                                v_out_1_best = v_out_1;
                                v_out_2_best = v_out_2;
                                c_best = c;
                            }
                            remove(solution, v_out_1);
                        }
                        assert(!contains(solution, v_in));
                        add(solution, v_in);
                    }
                    if (v_in_best != -1) {
                        improved = true;
                        // Apply move.
                        remove(solution, v_in_best);
                        assert(!contains(solution, v_out_1_best));
                        add(solution, v_out_1_best);
                        assert(!contains(solution, v_out_2_best));
                        add(solution, v_out_2_best);
                        assert(global_cost(solution) == c_best);
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
    }

    /*
     * Outputs.
     */

    std::ostream& print(
            std::ostream &os,
            const Solution& solution)
    {
        os << "vertices:";
        for (VertexId v = 0; v < instance_.number_of_vertices(); ++v)
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
        return solution.vertices[v].in;
    }

    inline void add(Solution& solution, VertexId v) const
    {
        assert(v >= 0);
        assert(!contains(solution, v));
        solution.vertices[v].in = true;
        Weight p = instance_.vertex(v).weight;
        solution.weight += p;
        for (const VertexEdge& edge: instance_.vertex(v).edges) {
            if (contains(solution, edge.v))
                remove(solution, edge.v);
            solution.vertices[edge.v].neighbor_weight += p;
        }
    }

    inline void remove(Solution& solution, VertexId v) const
    {
        assert(v >= 0);
        assert(contains(solution, v));
        solution.vertices[v].in = false;
        Weight p = instance_.vertex(v).weight;
        solution.weight -= p;
        for (const VertexEdge& edge: instance_.vertex(v).edges)
            solution.vertices[edge.v].neighbor_weight -= p;
    }

    /*
     * Evaluate moves.
     */

    inline GlobalCost cost_remove(const Solution& solution, VertexId v, GlobalCost) const
    {
        return {
            - (solution.weight - instance_.vertex(v).weight),
        };
    }

    inline GlobalCost cost_add(const Solution& solution, VertexId v, GlobalCost) const
    {
        return {
            -(solution.weight + instance_.vertex(v).weight - solution.vertices[v].neighbor_weight),
        };
    }

    /*
     * Private attributes.
     */

    const Instance& instance_;
    Parameters parameters_;

    std::vector<VertexId> vertices_;
    std::vector<VertexId> vertices_in_;
    std::vector<VertexId> vertices_out_;
    optimizationtools::IndexedSet neighbors_;
    optimizationtools::IndexedSet free_vertices_;
    optimizationtools::IndexedSet free_vertices_2_;

};

}

LocalSearchOutput& LocalSearchOutput::algorithm_end(
        optimizationtools::Info& info)
{
    //PUT(info, "Algorithm", "Iterations", iterations);
    Output::algorithm_end(info);
    //VER(info, "Iterations: " << iterations << std::endl);
    return *this;
}

LocalSearchOutput stablesolver::localsearch(
        const Instance& instance,
        std::mt19937_64&,
        LocalSearchOptionalParameters parameters)
{
    LocalSearchOutput output(instance, parameters.info);

    // Create LocalScheme.
    LocalScheme::Parameters parameters_local_scheme;
    LocalScheme local_scheme(instance, parameters_local_scheme);

    // Run A*.
    AStarLocalSearchOptionalParameters<LocalScheme> parameters_a_star;
    //parameters_a_star.info.set_verbose(true);
    parameters_a_star.info.set_time_limit(parameters.info.remaining_time());
    parameters_a_star.maximum_number_of_nodes = parameters.maximum_number_of_nodes;
    parameters_a_star.number_of_threads_1 = 1;
    parameters_a_star.number_of_threads_2 = parameters.number_of_threads;
    parameters_a_star.initial_solution_ids = std::vector<Counter>(
            parameters_a_star.number_of_threads_2, 0);
    parameters_a_star.new_solution_callback
        = [&instance, &parameters, &output](
                const LocalScheme::Solution& solution)
        {
            Solution sol(instance);
            for (VertexId v = 0; v < instance.number_of_vertices(); ++v)
                if (solution.vertices[v].in)
                    sol.add(v);
            std::stringstream ss;
            output.update_solution(sol, ss, parameters.info);
        };
    a_star_local_search(local_scheme, parameters_a_star);

    return output.algorithm_end(parameters.info);
}

