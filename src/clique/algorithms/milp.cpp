#include "stablesolver/clique/algorithms/milp.hpp"

#include "stablesolver/clique/algorithm_formatter.hpp"

using namespace stablesolver::clique;

namespace
{

mathoptsolverscmake::MilpModel create_milp_model(
        const Instance& instance)
{
    const optimizationtools::AbstractGraph* graph = instance.graph();

    int number_of_variables = graph->number_of_vertices() + 1;
    int number_of_constraints = graph->number_of_vertices() + 1;
    int number_of_elements = 0;

    // Variables
    mathoptsolverscmake::MilpModel model(
            number_of_variables,
            number_of_constraints,
            number_of_elements);

    // Variable and objective.
    model.objective_direction = mathoptsolverscmake::ObjectiveDirection::Maximize;
    for (VertexId vertex_id = 0;
            vertex_id < graph->number_of_vertices();
            ++vertex_id) {
        model.variables_lower_bounds[vertex_id] = 0;
        model.variables_upper_bounds[vertex_id] = 1;
        model.variables_types[vertex_id] = mathoptsolverscmake::VariableType::Binary;
        model.objective_coefficients[vertex_id] = graph->weight(vertex_id);
    }
    model.variables_lower_bounds[graph->number_of_vertices()] = 0;
    model.variables_upper_bounds[graph->number_of_vertices()] = graph->number_of_vertices();
    model.variables_types[graph->number_of_vertices()] = mathoptsolverscmake::VariableType::Integer;
    model.objective_coefficients[graph->number_of_vertices()] = 0;

    /////////////////
    // Constraints //
    /////////////////

    // Conflict constraints.
    for (VertexId vertex_id = 0;
            vertex_id < graph->number_of_vertices();
            ++vertex_id) {
        VertexId big_m = graph->number_of_vertices() - graph->degree(vertex_id) + 1;

        model.constraints_starts[vertex_id] = model.elements_variables.size();

        model.elements_variables.push_back(graph->number_of_vertices());
        model.elements_coefficients.push_back(1.0);
        for (auto it = graph->neighbors_begin(vertex_id);
                it != graph->neighbors_end(vertex_id);
                ++it) {
            model.elements_variables.push_back(*it);
            model.elements_coefficients.push_back(-1.0);
        }

        model.elements_variables.push_back(vertex_id);
        model.elements_coefficients.push_back(-1.0 + big_m);

        model.constraints_upper_bounds[vertex_id] = big_m;
    }

    // Linking constraints between x and z.
    {
        int constraint_id = graph->number_of_vertices();

        model.constraints_starts[constraint_id] = model.elements_variables.size();

        model.elements_variables.push_back(graph->number_of_vertices());
        model.elements_coefficients.push_back(1.0);
        for (VertexId vertex_id = 0;
                vertex_id < graph->number_of_vertices();
                ++vertex_id) {
            model.elements_variables.push_back(vertex_id);
            model.elements_coefficients.push_back(-1.0);
        }

        model.constraints_lower_bounds[constraint_id] = 0;
        model.constraints_upper_bounds[constraint_id] = 0;
    }

    return model;
}

Solution retrieve_solution(
        const Instance& instance,
        const std::vector<double>& milp_solution)
{
    const optimizationtools::AbstractGraph* graph = instance.graph();
    Solution solution(instance);
    for (VertexId vertex_id = 0;
            vertex_id < graph->number_of_vertices();
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

}

const Output stablesolver::clique::milp(
        const Instance& instance,
        const MilpParameters& parameters)
{
    Output output(instance);
    AlgorithmFormatter algorithm_formatter(parameters, output);
    algorithm_formatter.start("MILP (CPLEX)");

    algorithm_formatter.print_header();

    mathoptsolverscmake::MilpModel milp_model = create_milp_model(instance);

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
