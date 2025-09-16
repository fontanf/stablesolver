#include "stablesolver/stable/algorithms/milp.hpp"

#include "stablesolver/stable/algorithm_formatter.hpp"
#include "stablesolver/clique/algorithms/greedy.hpp"

using namespace stablesolver::stable;

namespace
{

mathoptsolverscmake::MilpModel create_milp_model_1(
        const Instance& instance)
{
    int number_of_variables = instance.number_of_vertices();
    int number_of_constraints = instance.number_of_edges();
    int number_of_elements = 2 * instance.number_of_edges();

    mathoptsolverscmake::MilpModel model(
            number_of_variables,
            number_of_constraints,
            number_of_elements);

    // Variable and objective.
    model.objective_direction = mathoptsolverscmake::ObjectiveDirection::Maximize;
    for (VertexId vertex_id = 0;
            vertex_id < instance.number_of_vertices();
            ++vertex_id) {
        const Vertex& vertex = instance.vertex(vertex_id);
        model.variables_lower_bounds[vertex_id] = 0;
        model.variables_upper_bounds[vertex_id] = 1;
        model.variables_types[vertex_id] = mathoptsolverscmake::VariableType::Binary;
        model.objective_coefficients[vertex_id] = vertex.weight;
    }

    // Constraints.
    int element_id = 0;
    int constraints_id = 0;

    // Constraints:edge constraints.
    for (VertexId vertex_id = 0;
            vertex_id < instance.number_of_vertices();
            ++vertex_id) {
        const Vertex& vertex = instance.vertex(vertex_id);
        for (const VertexEdge& edge: vertex.edges) {
            if (vertex_id > edge.vertex_id)
                continue;
            model.constraints_starts[constraints_id] = element_id;

            model.elements_variables[element_id] = vertex_id;
            model.elements_coefficients[element_id] = 1.0;
            element_id++;

            model.elements_variables[element_id] = edge.vertex_id;
            model.elements_coefficients[element_id] = 1.0;
            element_id++;

            model.constraints_upper_bounds[constraints_id] = 1;
            constraints_id++;
        }
    }

    return model;
}

mathoptsolverscmake::MilpModel create_milp_model_2(
        const Instance& instance)
{
    int number_of_variables = instance.number_of_vertices();
    int number_of_constraints = instance.number_of_vertices();
    int number_of_elements = instance.number_of_vertices() + 2 * instance.number_of_edges();

    mathoptsolverscmake::MilpModel model(
            number_of_variables,
            number_of_constraints,
            number_of_elements);

    // Variable and objective.
    model.objective_direction = mathoptsolverscmake::ObjectiveDirection::Maximize;
    for (VertexId vertex_id = 0;
            vertex_id < instance.number_of_vertices();
            ++vertex_id) {
        const Vertex& vertex = instance.vertex(vertex_id);
        model.variables_lower_bounds[vertex_id] = 0;
        model.variables_upper_bounds[vertex_id] = 1;
        model.variables_types[vertex_id] = mathoptsolverscmake::VariableType::Binary;
        model.objective_coefficients[vertex_id] = vertex.weight;
    }

    // Constraints.
    int element_id = 0;
    int constraints_id = 0;

    // Constraints:edge constraints.
    for (VertexId vertex_id = 0;
            vertex_id < instance.number_of_vertices();
            ++vertex_id) {
        const Vertex& vertex = instance.vertex(vertex_id);

        model.constraints_starts[constraints_id] = element_id;

        model.elements_variables[element_id] = vertex_id;
        model.elements_coefficients[element_id] = vertex.edges.size();
        element_id++;

        for (const VertexEdge& edge: vertex.edges) {
            if (vertex_id > edge.vertex_id)
                continue;
            model.elements_variables[element_id] = edge.vertex_id;
            model.elements_coefficients[element_id] = 1.0;
            element_id++;
        }

        model.constraints_upper_bounds[constraints_id] = vertex.edges.size();
        constraints_id++;
    }

    return model;
}

mathoptsolverscmake::MilpModel create_milp_model_3(
        const Instance& instance)
{
    mathoptsolverscmake::MilpModel model(instance.number_of_vertices(), 0, 0);

    // Variable and objective.
    model.objective_direction = mathoptsolverscmake::ObjectiveDirection::Maximize;
    for (VertexId vertex_id = 0;
            vertex_id < instance.number_of_vertices();
            ++vertex_id) {
        const Vertex& vertex = instance.vertex(vertex_id);
        model.variables_lower_bounds[vertex_id] = 0;
        model.variables_upper_bounds[vertex_id] = 1;
        model.variables_types[vertex_id] = mathoptsolverscmake::VariableType::Binary;
        model.objective_coefficients[vertex_id] = vertex.weight;
    }

    // Constraints
    optimizationtools::IndexedSet edge_set(instance.number_of_edges());
    optimizationtools::IndexedSet vertex_set_1(instance.number_of_vertices());
    optimizationtools::IndexedSet vertex_set_2(instance.number_of_vertices());
    std::vector<EdgeId> edge_indices(instance.number_of_edges(), 0);
    for (EdgeId e = 0; e < instance.number_of_edges(); ++e) {
        if (edge_set.contains(e))
            continue;
        // Compute vertices
        VertexId vertex_1_id = instance.edge(e).vertex_id_1;
        VertexId vertex_2_id = instance.edge(e).vertex_id_2;
        vertex_set_1.clear();
        vertex_set_2.clear();
        for (const auto& edge: instance.vertex(vertex_1_id).edges)
            if (edge.vertex_id != vertex_2_id)
                vertex_set_1.add(edge.vertex_id);
        for (const auto& edge: instance.vertex(vertex_2_id).edges)
            if (vertex_set_1.contains(edge.vertex_id))
                vertex_set_2.add(edge.vertex_id);
        optimizationtools::AdjacencyListGraphBuilder graph_builder;
        for (VertexId vertex_id = 0; vertex_id < vertex_set_2.size(); ++vertex_id)
            graph_builder.add_vertex();
        // Add edges
        //std::cout << "edge_id " << e << std::endl;
        //std::cout << "vertex_1_id " << vertex_1_id << " vertex_2_id " << vertex_2_id << std::endl;
        for (auto it = vertex_set_2.begin(); it != vertex_set_2.end(); ++it) {
            for (const auto& edge: instance.vertex(*it).edges) {
                //std::cout << "edge " << edge.edge_id << " " << *it << " " << edge.vertex_id << std::endl;
                if (edge.vertex_id > *it
                        && vertex_set_2.contains(edge.vertex_id)) {
                    //std::cout << "add" << std::endl;
                    EdgeId edge_id = graph_builder.add_edge(
                            vertex_set_2.position(*it),
                            vertex_set_2.position(edge.vertex_id));
                    edge_indices[edge_id] = edge.edge_id;
                }
            }
        }
        std::shared_ptr<optimizationtools::AbstractGraph> graph
            = std::shared_ptr<optimizationtools::AdjacencyListGraph>(
                    new optimizationtools::AdjacencyListGraph(graph_builder.build()));
        stablesolver::clique::Instance clique_instance(graph);
        // Solve
        stablesolver::clique::Parameters clique_parameters;
        clique_parameters.verbosity_level = 0;
        auto clique_output = stablesolver::clique::greedy_gwmin(
                clique_instance,
                clique_parameters);
        // Build constraint
        //std::cout << "new clique" << std::endl;
        model.constraints_starts.push_back(model.elements_variables.size());

        //std::cout << "add " << vertex_1_id << std::endl;
        //std::cout << "add " << vertex_2_id << std::endl;
        model.elements_variables.push_back(vertex_1_id);
        model.elements_coefficients.push_back(1.0);
        model.elements_variables.push_back(vertex_2_id);
        model.elements_coefficients.push_back(1.0);

        for (const auto& edge: instance.vertex(vertex_1_id).edges)
            if (vertex_set_2.contains(edge.vertex_id)
                    && clique_output.solution.contains(vertex_set_2.position(edge.vertex_id)))
                edge_set.add(edge.edge_id);
        for (const auto& edge: instance.vertex(vertex_2_id).edges)
            if (vertex_set_2.contains(edge.vertex_id)
                    && clique_output.solution.contains(vertex_set_2.position(edge.vertex_id)))
                edge_set.add(edge.edge_id);
        for (VertexId vertex_id_clique: clique_output.solution.vertices()) {
            VertexId vertex_id_orig = *(vertex_set_2.begin() + vertex_id_clique);
            //std::cout << "add " << vertex_id_orig << std::endl;
            model.elements_variables.push_back(vertex_id_orig);
            model.elements_coefficients.push_back(1);
            for (const auto& edge: clique_instance.adjacency_list_graph()->edges(vertex_id_clique)) {
                if (clique_output.solution.contains(edge.vertex_id)
                        && edge.vertex_id > vertex_id_clique) {
                    edge_set.add(edge_indices[edge.edge_id]);
                }
            }
        }

        model.constraints_lower_bounds.push_back(-std::numeric_limits<double>::infinity());
        model.constraints_upper_bounds.push_back(1);
    }

    return model;
}

Solution retrieve_solution(
        const Instance& instance,
        const std::vector<double>& milp_solution)
{
    Solution solution(instance);
    for (VertexId vertex_id = 0;
            vertex_id < instance.number_of_vertices();
            ++vertex_id) {
        if (milp_solution[vertex_id] > 0.5)
            solution.add(vertex_id);
    }
    return solution;
}

#ifdef CBC_FOUND

class EventHandler: public CbcEventHandler
{

public:

    virtual CbcAction event(CbcEvent which_event);

    EventHandler(
            const Instance& instance,
            const MilpParameters& parameters,
            const mathoptsolverscmake::MilpModel& milp_model,
            Output& output,
            AlgorithmFormatter& algorithm_formatter):
        CbcEventHandler(),
        instance_(instance),
        parameters_(parameters),
        milp_model_(milp_model),
        output_(output),
        algorithm_formatter_(algorithm_formatter) { }

    virtual ~EventHandler() { }

    EventHandler(const EventHandler &rhs):
        CbcEventHandler(rhs),
        instance_(rhs.instance_),
        parameters_(rhs.parameters_),
        milp_model_(rhs.milp_model_),
        output_(rhs.output_),
        algorithm_formatter_(rhs.algorithm_formatter_) { }

    virtual CbcEventHandler* clone() const { return new EventHandler(*this); }

private:

    const Instance& instance_;
    const MilpParameters& parameters_;
    const mathoptsolverscmake::MilpModel& milp_model_;
    Output& output_;
    AlgorithmFormatter& algorithm_formatter_;

};

CbcEventHandler::CbcAction EventHandler::event(CbcEvent which_event)
{
    // Not in subtree.
    if ((model_->specialOptions() & 2048) != 0)
        return noAction;
    const CbcModel& cbc_model = *model_;

    int number_of_nodes = mathoptsolverscmake::get_number_of_nodes(cbc_model);

    // Retrieve solution.
    double milp_objective_value = mathoptsolverscmake::get_solution_value(cbc_model);
    if (!output_.solution.feasible()
            || output_.solution.weight() < milp_objective_value) {
        std::vector<double> milp_solution = mathoptsolverscmake::get_solution(cbc_model);
        Solution solution = retrieve_solution(instance_, milp_solution);
        algorithm_formatter_.update_solution(solution, "node " + std::to_string(number_of_nodes));
    }

    // Retrieve bound.
    double milp_bound = mathoptsolverscmake::get_bound(cbc_model);
    if (milp_bound != std::numeric_limits<double>::infinity()) {
        algorithm_formatter_.update_bound(
                std::floor(milp_bound + 1e-5),
                "node " + std::to_string(number_of_nodes));
    }

    // Check end.
    if (parameters_.timer.needs_to_end())
        return stop;

    return noAction;
}

#endif

#ifdef XPRESS_FOUND

struct XpressCallbackUser
{
    const Instance& instance;
    const MilpParameters& parameters;
    Output& output;
    AlgorithmFormatter& algorithm_formatter;
};

void xpress_callback(
        XPRSprob xpress_model,
        void* user,
        int*)
{
    const XpressCallbackUser& d = *(const XpressCallbackUser*)(user);

    // Retrieve solution.
    double milp_objective_value = mathoptsolverscmake::get_solution_value(xpress_model);
    if (!d.output.solution.feasible()
            || ((d.instance.objective() == Objective::Makespan && d.output.solution.makespan() > milp_objective_value)
                || (d.instance.objective() == Objective::TotalFlowTime && d.output.solution.total_flow_time() > milp_objective_value)
                || (d.instance.objective() == Objective::TotalTardiness && d.output.solution.total_tardiness() > milp_objective_value))) {
        std::vector<double> milp_solution = mathoptsolverscmake::get_solution(xpress_model);
        Solution solution = retrieve_solution(d.instance, milp_solution);
        d.algorithm_formatter.update_solution(solution, "");
    }

    // Retrieve bound.
    Time bound = std::ceil(mathoptsolverscmake::get_bound(xpress_model) - 1e-5);
    if (d.instance.objective() == Objective::Makespan)
        d.algorithm_formatter.update_makespan_bound(bound, "");

    // Check end.
    if (d.parameters.timer.needs_to_end())
        XPRSinterrupt(xpress_model, XPRS_STOP_USER);
};

#endif

const Output milp(
        const Instance& instance,
        const MilpParameters& parameters,
        int model_id)
{
    Output output(instance);
    AlgorithmFormatter algorithm_formatter(parameters, output);
    algorithm_formatter.start("MILP " + std::to_string(model_id));

    // Reduction.
    if (parameters.reduction_parameters.reduce) {
        return solve_reduced_instance(
                [model_id](
                    const Instance& instance,
                    const Parameters& parameters)
                {
                    return milp(
                            instance,
                            static_cast<const MilpParameters&>(parameters),
                            model_id);
                },
                instance,
                parameters,
                algorithm_formatter,
                output);
    }

    algorithm_formatter.print_header();

    mathoptsolverscmake::MilpModel milp_model =
        (model_id == 1)? create_milp_model_1(instance):
        (model_id == 2)? create_milp_model_2(instance):
        create_milp_model_3(instance);

    std::vector<double> milp_solution;
    double milp_bound = 0;

    if (parameters.solver == mathoptsolverscmake::SolverName::Cbc) {
#ifdef CBC_FOUND
        OsiCbcSolverInterface osi_solver;
        CbcModel cbc_model(osi_solver);
        mathoptsolverscmake::reduce_printout(cbc_model);
        mathoptsolverscmake::set_time_limit(cbc_model, parameters.timer.remaining_time());
        mathoptsolverscmake::load(cbc_model, milp_model);
        EventHandler cbc_event_handler(instance, parameters, milp_model, output, algorithm_formatter);
        cbc_model.passInEventHandler(&cbc_event_handler);
        mathoptsolverscmake::solve(cbc_model);
        milp_solution = mathoptsolverscmake::get_solution(cbc_model);
        milp_bound = mathoptsolverscmake::get_bound(cbc_model);
#else
        throw std::invalid_argument("");
#endif

    } else if (parameters.solver == mathoptsolverscmake::SolverName::Highs) {
#ifdef HIGHS_FOUND
        Highs highs;
        mathoptsolverscmake::reduce_printout(highs);
        mathoptsolverscmake::set_time_limit(highs, parameters.timer.remaining_time());
        mathoptsolverscmake::set_log_file(highs, "highs.log");
        mathoptsolverscmake::load(highs, milp_model);
        highs.setCallback([
                &instance,
                &parameters,
                &milp_model,
                &output,
                &algorithm_formatter](
                    const int,
                    const std::string& message,
                    const HighsCallbackOutput* highs_output,
                    HighsCallbackInput* highs_input,
                    void*)
                {
                    if (!highs_output->mip_solution.empty()) {
                        // Retrieve solution.
                        double milp_objective_value = highs_output->mip_primal_bound;
                        if (!output.solution.feasible()
                                || output.solution.weight() < milp_objective_value) {
                            Solution solution = retrieve_solution(instance, highs_output->mip_solution);
                            algorithm_formatter.update_solution(solution, "node " + std::to_string(highs_output->mip_node_count));
                        }

                        // Retrieve bound.
                        double milp_bound = highs_output->mip_dual_bound;
                        if (milp_bound != std::numeric_limits<double>::infinity()) {
                            algorithm_formatter.update_bound(
                                    std::floor(milp_bound + 1e-5),
                                    "node " + std::to_string(highs_output->mip_node_count));
                        }
                    }

                    // Check end.
                    if (parameters.timer.needs_to_end())
                        highs_input->user_interrupt = 1;
                },
                nullptr);
        HighsStatus highs_status;
        highs_status = highs.startCallback(HighsCallbackType::kCallbackMipImprovingSolution);
        highs_status = highs.startCallback(HighsCallbackType::kCallbackMipInterrupt);
        mathoptsolverscmake::solve(highs);
        milp_solution = mathoptsolverscmake::get_solution(highs);
        milp_bound = mathoptsolverscmake::get_bound(highs);
#else
        throw std::invalid_argument("");
#endif

    } else if (parameters.solver == mathoptsolverscmake::SolverName::Xpress) {
#ifdef XPRESS_FOUND
        XPRSprob xpress_model;
        XPRScreateprob(&xpress_model);
        mathoptsolverscmake::set_time_limit(xpress_model, parameters.timer.remaining_time());
        mathoptsolverscmake::set_log_file(xpress_model, "xpress.log");
        mathoptsolverscmake::load(xpress_model, milp_model);
        //mathoptsolverscmake::write_mps(xpress_model, "kpc.mps");
        XpressCallbackUser xpress_callback_user{instance, parameters, output, algorithm_formatter};
        XPRSaddcbprenode(xpress_model, xpress_callback, (void*)&xpress_callback_user, 0);
        mathoptsolverscmake::solve(xpress_model);
        milp_solution = mathoptsolverscmake::get_solution(xpress_model);
        milp_bound = mathoptsolverscmake::get_bound(xpress_model);
        XPRSdestroyprob(xpress_model);
#else
        throw std::invalid_argument("");
#endif

    } else {
        throw std::invalid_argument("");
    }

    // Retrieve solution.
    Solution solution = retrieve_solution(instance, milp_solution);
    algorithm_formatter.update_solution(solution, "");

    // Retrieve bound.
    algorithm_formatter.update_bound(std::floor(milp_bound + 1e-5), "");

    algorithm_formatter.end();
    return output;
}

}

const Output stablesolver::stable::milp_1(
        const Instance& instance,
        const MilpParameters& parameters)
{
    return milp(instance, parameters, 1);
}

const Output stablesolver::stable::milp_2(
        const Instance& instance,
        const MilpParameters& parameters)
{
    return milp(instance, parameters, 2);
}

const Output stablesolver::stable::milp_3(
        const Instance& instance,
        const MilpParameters& parameters)
{
    return milp(instance, parameters, 3);
}
