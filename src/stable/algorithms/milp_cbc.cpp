#include "stablesolver/stable/algorithms/milp_cbc.hpp"

#include "stablesolver/stable/algorithm_formatter.hpp"

#include <coin/CbcModel.hpp>
#include <coin/OsiCbcSolverInterface.hpp>

using namespace stablesolver::stable;

class SolHandler: public CbcEventHandler
{

public:

    virtual CbcAction event(CbcEvent whichEvent);

    SolHandler(
            const Instance& instance,
            const MilpCbcParameters& parameters,
            Output& output,
            AlgorithmFormatter& algorithm_formatter):
        CbcEventHandler(),
        instance_(instance),
        parameters_(parameters),
        output_(output),
        algorithm_formatter_(algorithm_formatter) { }

    SolHandler(
            CbcModel *model,
            const Instance& instance,
            MilpCbcParameters& parameters,
            Output& output,
            AlgorithmFormatter& algorithm_formatter):
        CbcEventHandler(model),
        instance_(instance),
        parameters_(parameters),
        output_(output),
        algorithm_formatter_(algorithm_formatter) { }

    virtual ~SolHandler() { }

    SolHandler(const SolHandler &rhs):
        CbcEventHandler(rhs),
        instance_(rhs.instance_),
        parameters_(rhs.parameters_),
        output_(rhs.output_),
        algorithm_formatter_(rhs.algorithm_formatter_) { }

    SolHandler &operator=(const SolHandler &rhs)
    {
        if (this != &rhs) {
            CbcEventHandler::operator=(rhs);
            //this->instance_   = rhs.instance_;
            //this->parameters_ = rhs.parameters_;
            //this->output_ = rhs.output_;
            //this->algorithm_formatter_ = rhs.algorithm_formatter_;
        }
        return *this;
    }

    virtual CbcEventHandler *clone() const { return new SolHandler(*this); }

private:

    const Instance& instance_;
    const MilpCbcParameters& parameters_;
    Output& output_;
    AlgorithmFormatter& algorithm_formatter_;

};

CbcEventHandler::CbcAction SolHandler::event(CbcEvent whichEvent)
{
    if ((model_->specialOptions() & 2048) != 0) // not in subtree
        return noAction;

    Weight ub = -model_->getBestPossibleObjValue();
    algorithm_formatter_.update_bound(ub, "");

    if ((whichEvent != solution && whichEvent != heuristicSolution)) // no solution found
        return noAction;

    OsiSolverInterface *origSolver = model_->solver();
    const OsiSolverInterface *pps = model_->postProcessedSolver(1);
    const OsiSolverInterface *solver = pps? pps: origSolver;

    if (!output_.solution.feasible()
            || output_.solution.weight() < -solver->getObjValue()) {
        const double *solution_cbc = solver->getColSolution();
        Solution solution(instance_);
        for (VertexId vertex_id = 0;
                vertex_id < instance_.number_of_vertices();
                ++vertex_id) {
            if (solution_cbc[vertex_id] > 0.5)
                solution.add(vertex_id);
        }
        algorithm_formatter_.update_solution(
                solution,
                "");
    }

    return noAction;
}

////////////////////////////////////////////////////////////////////////////////
/////////////////////////// Model 1, |E| constraints ///////////////////////////
////////////////////////////////////////////////////////////////////////////////

const Output stablesolver::stable::milp_1_cbc(
        const Instance& instance,
        const MilpCbcParameters& parameters)
{
    Output output(instance);
    AlgorithmFormatter algorithm_formatter(parameters, output);
    algorithm_formatter.start("MILP 1 (Cbc)");

    // Reduction.
    if (parameters.reduction_parameters.reduce)
        return solve_reduced_instance(milp_1_cbc, instance, parameters, algorithm_formatter, output);

    algorithm_formatter.print_header();

    // Variables
    int number_of_columns = instance.number_of_vertices();
    std::vector<double> colum_lower_bounds(number_of_columns, 0);
    std::vector<double> colum_upper_bounds(number_of_columns, 1);

    // Objective
    std::vector<double> objective(number_of_columns);
    for (VertexId vertex_id = 0;
            vertex_id < instance.number_of_vertices();
            ++vertex_id) {
        objective[vertex_id] = instance.vertex(vertex_id).weight;
    }

    // Constraints
    int number_of_rows = 0; // will be increased each time we add a constraint
    std::vector<CoinBigIndex> row_starts;
    std::vector<int> number_of_elements_in_rows;
    std::vector<int> element_columns;
    std::vector<double> elements;
    std::vector<double> row_lower_bounds;
    std::vector<double> row_upper_bounds;

    for (EdgeId edge_id = 0; edge_id < instance.number_of_edges(); ++edge_id) {
        row_starts.push_back(elements.size());
        number_of_elements_in_rows.push_back(0);
        number_of_rows++;
        element_columns.push_back(instance.edge(edge_id).vertex_id_1);
        element_columns.push_back(instance.edge(edge_id).vertex_id_2);
        elements.push_back(1);
        elements.push_back(1);
        number_of_elements_in_rows.back()++;
        number_of_elements_in_rows.back()++;
        row_lower_bounds.push_back(0);
        row_upper_bounds.push_back(1);
    }

    // Create matrix
    row_starts.push_back(elements.size());
    CoinPackedMatrix matrix(
            false,
            number_of_columns,
            number_of_rows,
            elements.size(),
            elements.data(),
            element_columns.data(),
            row_starts.data(),
            number_of_elements_in_rows.data());

    OsiCbcSolverInterface solver1;

    // Reduce printout
    solver1.getModelPtr()->setLogLevel(0);
    solver1.messageHandler()->setLogLevel(0);

    // Load problem
    solver1.loadProblem(
            matrix,
            colum_lower_bounds.data(),
            colum_upper_bounds.data(),
            objective.data(),
            row_lower_bounds.data(),
            row_upper_bounds.data());

    // Mark integer
    for (VertexId vertex_id = 0;
            vertex_id < instance.number_of_vertices();
            ++vertex_id) {
        solver1.setInteger(vertex_id);
    }

    // Pass data and solver to CbcModel
    CbcModel model(solver1);

    // Maximize.
    model.setObjSense(-1);

    // Callback
    SolHandler sh(instance, parameters, output, algorithm_formatter);
    model.passInEventHandler(&sh);

    // Reduce printout
    model.setLogLevel(0);
    model.solver()->setHintParam(OsiDoReducePrint, true, OsiHintTry);

    // Set time limit
    model.setMaximumSeconds(parameters.timer.remaining_time());

    // Do complete search
    model.branchAndBound();

    if (model.isProvenInfeasible()) {
        throw std::logic_error("");
    } else if (model.isProvenOptimal()) {
        if (!output.solution.feasible()
                || output.solution.weight() < -model.getObjValue()) {
            const double *solution_cbc = model.solver()->getColSolution();
            Solution solution(instance);
            for (VertexId vertex_id = 0;
                    vertex_id < instance.number_of_vertices();
                    ++vertex_id) {
                if (solution_cbc[vertex_id] > 0.5)
                    solution.add(vertex_id);
            }
            algorithm_formatter.update_solution(
                    solution,
                    "");
        }
        algorithm_formatter.update_bound(
                output.solution.weight(),
                "");
    } else if (model.bestSolution() != NULL) {
        if (!output.solution.feasible()
                || output.solution.weight() < -model.getObjValue()) {
            const double *solution_cbc = model.solver()->getColSolution();
            Solution solution(instance);
            for (VertexId vertex_id = 0;
                    vertex_id < instance.number_of_vertices();
                    ++vertex_id) {
                if (solution_cbc[vertex_id] > 0.5)
                    solution.add(vertex_id);
            }
            algorithm_formatter.update_solution(
                    solution,
                    "");
        }
        Weight ub = -model.getBestPossibleObjValue();
        algorithm_formatter.update_bound(ub, "");
    } else {
        Weight ub = -model.getBestPossibleObjValue();
        algorithm_formatter.update_bound(ub, "");
    }

    algorithm_formatter.end();
    return output;
}
