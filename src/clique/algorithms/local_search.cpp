#include "stablesolver/clique/algorithms/local_search.hpp"

#include "stablesolver/clique/algorithm_formatter.hpp"
#include "stablesolver/clique/algorithms/greedy.hpp"

#include "optimizationtools/containers/indexed_set.hpp"
#include "optimizationtools/containers/indexed_map.hpp"

#include "localsearchsolver/best_first_local_search.hpp"

using namespace stablesolver::clique;

namespace
{

class LocalScheme
{

public:

    /*
     * Constructors and destructor
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
        relevant_vertices_(instance.graph()->number_of_vertices()),
        tight_vertices_(instance.graph()->number_of_vertices(), -1),
        neighbors_(instance_.graph()->number_of_vertices()),
        neighbors_2_(instance_.graph()->number_of_vertices())
    {
        // Initialize relevant_vertices_.
        relevant_vertices_.fill();
    }

    /*
     * Global cost
     */

    /** Global cost: <Weight>; */
    using GlobalCost = std::tuple<Weight>;

    inline Weight&       weight(GlobalCost& global_cost) { return std::get<0>(global_cost); }
    inline Weight  weight(const GlobalCost& global_cost) { return std::get<0>(global_cost); }

    /*
     * Solutions
     */

    struct Solution
    {
        Solution(VertexId number_of_vertices):
            vertices(number_of_vertices),
            addition_costs(number_of_vertices, 0) {  }

        optimizationtools::IndexedSet vertices;
        std::vector<Weight> addition_costs;
        Weight weight = 0;
    };

    inline Solution empty_solution() const
    {
        return Solution(instance_.graph()->number_of_vertices());
    }

    inline Solution initial_solution(
            Counter,
            std::mt19937_64&)
    {
        stablesolver::clique::Parameters greedy_parameters;
        greedy_parameters.verbosity_level = 0;
        auto output_greedy = greedy_gwmin(instance_, greedy_parameters);
        Solution solution = empty_solution();
        for (VertexId vertex_id: relevant_vertices_)
            if (output_greedy.solution.contains(vertex_id))
                add(solution, vertex_id);
        return solution;
    }

    inline GlobalCost global_cost(const Solution& solution) const
    {
        return {
            -solution.weight,
        };
    }

    /*
     * Local search
     */

    struct Perturbation;

    inline void local_search(
            Solution& solution,
            std::mt19937_64& generator,
            const Perturbation& tabu = Perturbation())
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
            for (VertexId vertex_id: solution.vertices) {
                if (!relevant_vertices_.contains(vertex_id)) {
                    //std::cout << "remove " << v << std::endl;
                    remove(solution, vertex_id);
                }
            }
        }

        Counter it = 0;
        (void)it;
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
                    VertexId vertex_id_best = -1;
                    GlobalCost c_best = global_cost(solution);
                    for (VertexId vertex_id: relevant_vertices_) {
                        if (vertex_id == tabu.vertex_id)
                            continue;
                        if (contains(solution, vertex_id))
                            continue;
                        GlobalCost c = cost_add(solution, vertex_id, c_best);
                        if (c >= c_best)
                            continue;
                        if (vertex_id_best != -1
                                && !localsearchsolver::dominates(c, c_best)) {
                            continue;
                        }
                        vertex_id_best = vertex_id;
                        c_best = c;
                    }
                    //std::cout << "vertex_id_best " << vertex_id_best << std::endl;
                    if (vertex_id_best != -1) {
                        improved = true;
                        // Apply perturbation.
                        assert(!contains(solution, vertex_id_best));
                        add(solution, vertex_id_best);
                        if (global_cost(solution) != c_best) {
                            throw std::logic_error(
                                    "Add. Costs do not match:\n"
                                    "* vertex_id_best: " + std::to_string(vertex_id_best) + "\n"
                                    + "* Expected new cost: " + localsearchsolver::to_string(c_best) + "\n"
                                    + "* Actual new cost: " + localsearchsolver::to_string(global_cost(solution)) + "\n");
                        }
                    }
                    break;
                } case 1: { // (2-1)-swap neighborhood.
                    // Find vertices connected to all but one vertex of the
                    // current solution.
                    tight_vertices_.clear();
                    relevant_vertices_.shuffle_in(generator);
                    for (VertexId vertex_id: relevant_vertices_) {
                        if (vertex_id == tabu.vertex_id)
                            continue;
                        if (contains(solution, vertex_id))
                            continue;
                        VertexId vertex_id_2 = -1;
                        for (auto it = instance_.graph()->neighbors_begin(vertex_id);
                                it != instance_.graph()->neighbors_end(vertex_id);
                                ++it) {
                            if (!contains(solution, *it)) {
                                if (vertex_id_2 != -1) {
                                    vertex_id_2 = -1;
                                    break;
                                }
                                vertex_id_2 = *it;
                            }
                        }
                        if (vertex_id_2 != -1) {
                            tight_vertices_.set(vertex_id, vertex_id_2);
                        }
                    }

                    VertexId vertex_id_in_best = -1;
                    VertexId vertex_id_out_1_best = -1;
                    VertexId vertex_id_out_2_best = -1;
                    GlobalCost c_best = global_cost(solution);
                    for (const auto& p: tight_vertices_) {
                        VertexId vertex_id_out_1 = p.first;
                        VertexId vertex_id_in = p.second;
                        for (auto it = instance_.graph()->neighbors_begin(vertex_id_out_1);
                                it != instance_.graph()->neighbors_end(vertex_id_out_1);
                                ++it) {
                            VertexId vertex_id_out_2 = *it;
                            if (tight_vertices_.contains(vertex_id_out_2)
                                    && tight_vertices_[vertex_id_out_2] == vertex_id_in) {
                                GlobalCost c = -(solution.weight
                                        + instance_.graph()->weight(vertex_id_out_1)
                                        + instance_.graph()->weight(vertex_id_out_2)
                                        - instance_.graph()->weight(vertex_id_in));
                                if (c >= c_best)
                                    continue;
                                if (vertex_id_in_best != -1
                                        && !localsearchsolver::dominates(c, c_best)) {
                                    continue;
                                }
                                vertex_id_in_best = vertex_id_in;
                                vertex_id_out_1_best = vertex_id_out_1;
                                vertex_id_out_2_best = vertex_id_out_2;
                                c_best = c;
                            }
                        }
                    }
                    if (vertex_id_in_best != -1) {
                        improved = true;
                        // Apply perturbation.
                        assert(contains(solution, vertex_id_in_best));
                        remove(solution, vertex_id_in_best);
                        assert(!contains(solution, vertex_id_out_1_best));
                        add(solution, vertex_id_out_1_best);
                        assert(!contains(solution, vertex_id_out_2_best));
                        add(solution, vertex_id_out_2_best);
                        if (global_cost(solution) != c_best) {
                            throw std::logic_error(
                                    "(2,1)-swap. Costs do not match:\n"
                                    "* Expected new cost: " + localsearchsolver::to_string(c_best) + "\n"
                                    + "* Actual new cost: " + localsearchsolver::to_string(global_cost(solution)) + "\n");
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
     * Iterated local search
     */

    struct Perturbation
    {
        Perturbation(): vertex_id(-1) { }

        VertexId vertex_id;
        GlobalCost global_cost;
    };

    inline std::vector<Perturbation> perturbations(
            const Solution& solution,
            std::mt19937_64&)
    {
        std::vector<Perturbation> perturbations;
        for (VertexId vertex_id: relevant_vertices_) {
            GlobalCost c = (contains(solution, vertex_id))?
                cost_remove(solution, vertex_id, localsearchsolver::worst<GlobalCost>()):
                cost_add(solution, vertex_id, localsearchsolver::worst<GlobalCost>());
            Perturbation perturbation;
            perturbation.vertex_id = vertex_id;
            perturbation.global_cost = c;
            perturbations.push_back(perturbation);
        }
        return perturbations;
    }

    inline void apply_perturbation(
            Solution& solution,
            const Perturbation& perturbation,
            std::mt19937_64&) const
    {
        if (contains(solution, perturbation.vertex_id)) {
            remove(solution, perturbation.vertex_id);
        } else {
            if (relevant_vertices_.contains(perturbation.vertex_id))
                add(solution, perturbation.vertex_id);
        }
    }

    /*
     * Best first local search
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

    CompactSolution solution2compact(const Solution& solution)
    {
        std::vector<bool> vertices(instance_.graph()->number_of_vertices(), false);
        for (VertexId vertex_id: solution.vertices)
            vertices[vertex_id] = true;
        return vertices;
    }

    Solution compact2solution(const CompactSolution& compact_solution)
    {
        auto solution = empty_solution();
        for (VertexId vertex_id = 0;
                vertex_id < instance_.graph()->number_of_vertices();
                ++vertex_id) {
            if (relevant_vertices_.contains(vertex_id))
                if (compact_solution[vertex_id])
                    add(solution, vertex_id);
        }
        return solution;
    }

    struct PerturbationHasher
    {
        std::hash<VertexId> hasher;

        inline bool hashable(const Perturbation&) const { return true; }

        inline bool operator()(
                const Perturbation& perturbation_1,
                const Perturbation& perturbation_2) const
        {
            return perturbation_1.vertex_id == perturbation_2.vertex_id;
        }

        inline std::size_t operator()(
                const Perturbation& perturbation) const
        {
            size_t hash = hasher(perturbation.vertex_id);
            return hash;
        }
    };

    inline PerturbationHasher perturbation_hasher() const { return PerturbationHasher(); }

    /*
     * Output
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
        for (VertexId vertex_id = 0;
                vertex_id < instance_.graph()->number_of_vertices();
                ++vertex_id)
            if (contains(solution, vertex_id))
                os << " " << vertex_id;
        os << std::endl;
        os << "weight: " << solution.weight << std::endl;
        return os;
    }

    inline void write(const Solution&, std::string) const { return; }

private:

    /*
     * Manipulate solutions
     */

    inline bool contains(
            const Solution& solution,
            VertexId vertex_id) const
    {
        return solution.vertices.contains(vertex_id);
    }

    inline void add(
            Solution& solution,
            VertexId vertex_id) const
    {
        assert(vertex_id >= 0);
        assert(!contains(solution, vertex_id));
        assert(relevant_vertices_.contains(vertex_id));
        //std::cout << "add " << v
        //    << " c " << solution.addition_costs[v]
        //    << " rv " << relevant_vertices_.contains(v)
        //    << std::endl;

        neighbors_.clear();
        neighbors_.add(vertex_id);
        for (auto it = instance_.graph()->neighbors_begin(vertex_id);
                it != instance_.graph()->neighbors_end(vertex_id);
                ++it) {
            neighbors_.add(*it);
        }

        Weight weight = instance_.graph()->weight(vertex_id);

        // Reperturbation conflicting vertices.
        for (VertexId vertex_id_2: relevant_vertices_) {
            if (!neighbors_.contains(vertex_id_2)) {
                if (contains(solution, vertex_id_2))
                    remove(solution, vertex_id_2);
                solution.addition_costs[vertex_id_2] += weight;
            }
        }

        solution.vertices.add(vertex_id);
        solution.weight += weight;
        assert(solution.addition_costs[vertex_id] == 0);
    }

    inline void remove(
            Solution& solution,
            VertexId vertex_id) const
    {
        assert(vertex_id >= 0);
        assert(contains(solution, vertex_id));
        //std::cout << "remove " << v
        //    << " rv " << relevant_vertices_.contains(v)
        //    << std::endl;

        neighbors_2_.clear();
        neighbors_2_.add(vertex_id);
        for (auto it = instance_.graph()->neighbors_begin(vertex_id);
                it != instance_.graph()->neighbors_end(vertex_id);
                ++it) {
            neighbors_2_.add(*it);
        }

        Weight weight = instance_.graph()->weight(vertex_id);

        solution.vertices.remove(vertex_id);
        solution.weight -= weight;
        for (VertexId vertex_id_2: relevant_vertices_) {
            if (!neighbors_2_.contains(vertex_id_2)) {
                solution.addition_costs[vertex_id_2] -= weight;
                assert(solution.addition_costs[vertex_id_2] >= 0);
            }
        }
    }

    /*
     * Evaluate perturbations
     */

    inline GlobalCost cost_remove(
            const Solution& solution,
            VertexId vertex_id,
            GlobalCost) const
    {
        return {
            - (solution.weight - instance_.graph()->weight(vertex_id)),
        };
    }

    inline GlobalCost cost_add(
            const Solution& solution,
            VertexId vertex_id,
            GlobalCost) const
    {
        return {
            -(solution.weight
                    + instance_.graph()->weight(vertex_id)
                    - solution.addition_costs[vertex_id]),
        };
    }

    /*
     * Private attributes
     */

    const Instance& instance_;
    Parameters parameters_;
    Weight best_weight_ = 0;

    optimizationtools::IndexedSet relevant_vertices_;
    optimizationtools::IndexedMap<VertexId> tight_vertices_;
    mutable optimizationtools::IndexedSet neighbors_;
    mutable optimizationtools::IndexedSet neighbors_2_;

};

}

const Output stablesolver::clique::local_search(
        const Instance& instance,
        std::mt19937_64&,
        const LocalSearchParameters& parameters)
{
    Output output(instance);
    AlgorithmFormatter algorithm_formatter(parameters, output);
    algorithm_formatter.start("Local search");
    algorithm_formatter.print_header();

    // Create LocalScheme.
    LocalScheme::Parameters parameters_local_scheme;
    LocalScheme local_scheme(instance, parameters_local_scheme);

    // Run A*.
    localsearchsolver::BestFirstLocalSearchParameters<LocalScheme> llsbfls_parameters;
    llsbfls_parameters.verbosity_level = 0;
    llsbfls_parameters.timer = parameters.timer;
    llsbfls_parameters.maximum_number_of_nodes = parameters.maximum_number_of_nodes;
    llsbfls_parameters.number_of_threads_1 = 1;
    llsbfls_parameters.number_of_threads_2 = parameters.number_of_threads;
    llsbfls_parameters.initial_solution_ids = std::vector<Counter>(
            llsbfls_parameters.number_of_threads_2, 0);
    bool end = false;
    llsbfls_parameters.timer.add_end_boolean(&end);
    llsbfls_parameters.new_solution_callback
        = [&instance, &output, &algorithm_formatter, &end](
                const localsearchsolver::Output<LocalScheme>& lss_output)
        {
            Solution solution(instance);
            for (VertexId v = 0; v < instance.graph()->number_of_vertices(); ++v)
                if (lss_output.solution_pool.best().vertices.contains(v))
                    solution.add(v);
            algorithm_formatter.update_solution(solution, "");

            optimizationtools::IndexedSet relevant_vertices(instance.graph()->number_of_vertices());
            relevant_vertices.fill();
            Weight bound = instance.update_core(
                    relevant_vertices,
                    output.solution.weight());
            algorithm_formatter.update_bound(bound, "");

            if (output.optimal())
                end = true;
        };
    localsearchsolver::best_first_local_search(local_scheme, llsbfls_parameters);

    algorithm_formatter.end();
    return output;
}
