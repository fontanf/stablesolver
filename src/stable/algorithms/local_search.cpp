#include "stablesolver/stable/algorithms/local_search.hpp"

#include "stablesolver/stable/algorithm_formatter.hpp"

#include "localsearchsolver/best_first_local_search.hpp"

using namespace stablesolver::stable;

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
        vertices_(instance.number_of_vertices()),
        neighbors_(instance_.number_of_vertices()),
        free_vertices_(instance_.number_of_vertices()),
        free_vertices_2_(instance_.number_of_vertices())
    {
        // Initialize vertices_.
        std::iota(vertices_.begin(), vertices_.end(), 0);
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

    struct SolutionVertex
    {
        /**
         * in == true iff the vertex is in the solution.
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
        for (VertexId vertex_id: vertices_)
            if (solution.vertices[vertex_id].neighbor_weight == 0)
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
     * Local search.
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
                    std::shuffle(vertices_.begin(), vertices_.end(), generator);
                    VertexId vertex_id_best = -1;
                    GlobalCost c_best = global_cost(solution);
                    for (VertexId vertex_id: vertices_) {
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
                    if (vertex_id_best != -1) {
                        improved = true;
                        // Apply perturbation.
                        add(solution, vertex_id_best);
                        if (global_cost(solution) != c_best) {
                            throw std::logic_error(
                                    "Add. Costs do not match:\n"
                                    "* Expected new cost: " + localsearchsolver::to_string(c_best) + "\n"
                                    + "* Actual new cost: " + localsearchsolver::to_string(global_cost(solution)) + "\n");
                        }
                    }
                    break;
                } case 1: { // (2-1)-swap neighborhood.
                    std::shuffle(vertices_.begin(), vertices_.end(), generator);
                    // Get the vertices of the solution.
                    vertices_in_.clear();
                    for (VertexId vertex_id: vertices_) {
                        if (vertex_id == tabu.vertex_id)
                            continue;
                        if (contains(solution, vertex_id))
                            vertices_in_.push_back(vertex_id);
                    }

                    VertexId vertex_id_in_best = -1;
                    VertexId vertex_id_out_1_best = -1;
                    VertexId vertex_id_out_2_best = -1;
                    GlobalCost c_best = global_cost(solution);
                    for (VertexId vertex_id_in: vertices_in_) {
                        // Update free_vertices_
                        free_vertices_.clear();
                        for (const VertexEdge& edge: instance_.vertex(vertex_id_in).edges)
                            if (solution.vertices[edge.vertex_id].neighbor_weight
                                    == instance_.vertex(vertex_id_in).weight)
                                free_vertices_.add(edge.vertex_id);
                        if (free_vertices_.size() <= 2)
                            continue;
                        free_vertices_.shuffle_in(generator);
                        remove(solution, vertex_id_in);
                        for (VertexId vertex_id_out_1: free_vertices_) {
                            assert(vertex_id_out_1 != vertex_id_in);
                            free_vertices_2_.clear();
                            for (VertexId v: free_vertices_)
                                free_vertices_2_.add(v);
                            free_vertices_2_.remove(vertex_id_out_1);
                            for (const VertexEdge& edge: instance_.vertex(vertex_id_out_1).edges)
                                if (free_vertices_2_.contains(edge.vertex_id))
                                    free_vertices_2_.remove(edge.vertex_id);
                            if (free_vertices_2_.empty())
                                continue;
                            free_vertices_2_.shuffle_in(generator);
                            assert(!contains(solution, vertex_id_out_1));
                            add(solution, vertex_id_out_1);
                            for (VertexId vertex_id_out_2: free_vertices_2_) {
                                assert(vertex_id_out_2 != vertex_id_in);
                                assert(vertex_id_out_2 != vertex_id_out_1);
                                GlobalCost c = cost_add(solution, vertex_id_out_2, c_best);
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
                            remove(solution, vertex_id_out_1);
                        }
                        assert(!contains(solution, vertex_id_in));
                        add(solution, vertex_id_in);
                    }
                    if (vertex_id_in_best != -1) {
                        improved = true;
                        // Apply perturbation.
                        remove(solution, vertex_id_in_best);
                        assert(!contains(solution, vertex_id_out_1_best));
                        add(solution, vertex_id_out_1_best);
                        assert(!contains(solution, vertex_id_out_2_best));
                        add(solution, vertex_id_out_2_best);
                        if (global_cost(solution) != c_best) {
                            throw std::logic_error(
                                    "(2,1-swap). Costs do not match:\n"
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
    }

    /*
     * Iterated local search.
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
        for (VertexId vertex_id: vertices_) {
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
            add(solution, perturbation.vertex_id);
        }
    }

    /*
     * Best first local search.
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
        std::vector<bool> vertices(instance_.number_of_vertices(), false);
        for (VertexId vertex_id = 0;
                vertex_id < instance_.number_of_vertices();
                ++vertex_id) {
            if (solution.vertices[vertex_id].in)
                vertices[vertex_id] = true;
        }
        return vertices;
    }

    Solution compact2solution(const CompactSolution& compact_solution)
    {
        auto solution = empty_solution();
        for (VertexId vertex_id = 0;
                vertex_id < instance_.number_of_vertices();
                ++vertex_id) {
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
     * Outputs
     */

    std::ostream& print(
            std::ostream& os,
            const Solution& solution)
    {
        os << "vertices:";
        for (VertexId vertex_id = 0;
                vertex_id < instance_.number_of_vertices();
                ++vertex_id) {
            if (contains(solution, vertex_id))
                os << " " << vertex_id;
        }
        os << std::endl;
        os << "weight: " << solution.weight << std::endl;
        return os;
    }

    inline void write(const Solution&, std::string) const { return; }

private:

    /*
     * Manipulate solutions.
     */

    inline bool contains(
            const Solution& solution,
            VertexId vertex_id) const
    {
        return solution.vertices[vertex_id].in;
    }

    inline void add(
            Solution& solution,
            VertexId vertex_id) const
    {
        assert(vertex_id >= 0);
        assert(!contains(solution, vertex_id));

        Weight weight = instance_.vertex(vertex_id).weight;

        // Reperturbation conflicting vertices.
        for (const VertexEdge& edge: instance_.vertex(vertex_id).edges) {
            if (contains(solution, edge.vertex_id))
                remove(solution, edge.vertex_id);
            solution.vertices[edge.vertex_id].neighbor_weight += weight;
        }

        solution.vertices[vertex_id].in = true;
        solution.weight += weight;
    }

    inline void remove(
            Solution& solution,
            VertexId vertex_id) const
    {
        assert(vertex_id >= 0);
        assert(contains(solution, vertex_id));

        solution.vertices[vertex_id].in = false;
        Weight weight = instance_.vertex(vertex_id).weight;
        solution.weight -= weight;
        for (const VertexEdge& edge: instance_.vertex(vertex_id).edges)
            solution.vertices[edge.vertex_id].neighbor_weight -= weight;
    }

    /*
     * Evaluate perturbations.
     */

    inline GlobalCost cost_remove(
            const Solution& solution,
            VertexId vertex_id,
            GlobalCost) const
    {
        return {
            - (solution.weight - instance_.vertex(vertex_id).weight),
        };
    }

    inline GlobalCost cost_add(
            const Solution& solution,
            VertexId vertex_id,
            GlobalCost) const
    {
        return {
            -(solution.weight
                    + instance_.vertex(vertex_id).weight
                    - solution.vertices[vertex_id].neighbor_weight),
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

const Output stablesolver::stable::local_search(
        const Instance& instance,
        const LocalSearchParameters& parameters)
{
    Output output(instance);
    AlgorithmFormatter algorithm_formatter(parameters, output);
    algorithm_formatter.start("Local search");

    // Reduction.
    if (parameters.reduction_parameters.reduce)
        return solve_reduced_instance(local_search, instance, parameters, algorithm_formatter, output);

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
    llsbfls_parameters.new_solution_callback
        = [&instance, &algorithm_formatter](
                const localsearchsolver::Output<LocalScheme>& ls_output)
        {
            Solution solution(instance);
            for (VertexId vertex_id = 0;
                    vertex_id < instance.number_of_vertices();
                    ++vertex_id) {
                if (ls_output.solution_pool.best().vertices[vertex_id].in)
                    solution.add(vertex_id);
            }
            algorithm_formatter.update_solution(solution, "");
        };
    best_first_local_search(local_scheme, llsbfls_parameters);

    algorithm_formatter.end();
    return output;
}
