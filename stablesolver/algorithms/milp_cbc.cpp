#if COINOR_FOUND

#include "stablesolver/algorithms/milp_cbc.hpp"

#include <coin/CbcModel.hpp>
#include <coin/OsiCbcSolverInterface.hpp>

using namespace stablesolver;

MilpCbcOutput& MilpCbcOutput::algorithm_end(
        optimizationtools::Info& info)
{
    //info.add_to_json("Algorithm", "Iterations", it);
    Output::algorithm_end(info);
    return *this;
}

class SolHandler: public CbcEventHandler
{

public:

    virtual CbcAction event(CbcEvent whichEvent);

    SolHandler(
            const Instance& instance,
            MilpCbcOptionalParameters& parameters,
            MilpCbcOutput& output):
        CbcEventHandler(),
        instance_(instance),
        parameters_(parameters),
        output_(output) { }

    SolHandler(
            CbcModel *model,
            const Instance& instance,
            MilpCbcOptionalParameters& parameters,
            MilpCbcOutput& output):
        CbcEventHandler(model),
        instance_(instance),
        parameters_(parameters),
        output_(output) { }

    virtual ~SolHandler() { }

    SolHandler(const SolHandler &rhs):
        CbcEventHandler(rhs),
        instance_(rhs.instance_),
        parameters_(rhs.parameters_),
        output_(rhs.output_) { }

    SolHandler &operator=(const SolHandler &rhs)
    {
        if (this != &rhs) {
            CbcEventHandler::operator=(rhs);
            //this->instance_   = rhs.instance_;
            this->parameters_ = rhs.parameters_;
            this->output_     = rhs.output_;
        }
        return *this;
    }

    virtual CbcEventHandler *clone() const { return new SolHandler(*this); }

private:

    const Instance& instance_;
    MilpCbcOptionalParameters& parameters_;
    MilpCbcOutput& output_;

};

CbcEventHandler::CbcAction SolHandler::event(CbcEvent whichEvent)
{
    if ((model_->specialOptions() & 2048) != 0) // not in subtree
        return noAction;

    Weight ub = -model_->getBestPossibleObjValue();
    output_.update_upper_bound(ub, std::stringstream(""), parameters_.info);

    if ((whichEvent != solution && whichEvent != heuristicSolution)) // no solution found
        return noAction;

    OsiSolverInterface *origSolver = model_->solver();
    const OsiSolverInterface *pps = model_->postProcessedSolver(1);
    const OsiSolverInterface *solver = pps? pps: origSolver;

    if (!output_.solution.feasible()
            || output_.solution.weight() < -solver->getObjValue()) {
        const double *solution_cbc = solver->getColSolution();
        Solution solution(instance_);
        for (VertexId v = 0; v < instance_.number_of_vertices(); ++v)
            if (solution_cbc[v] > 0.5)
                solution.add(v);
        output_.update_solution(solution, std::stringstream(""), parameters_.info);
    }

    return noAction;
}

////////////////////////////////////////////////////////////////////////////////
/////////////////////////// Model 1, |E| constraints ///////////////////////////
////////////////////////////////////////////////////////////////////////////////

MilpCbcOutput stablesolver::milp_1_cbc(
        const Instance& instance,
        MilpCbcOptionalParameters parameters)
{
    init_display(instance, parameters.info);
    parameters.info.os()
        << "Algorithm" << std::endl
        << "---------" << std::endl
        << "MILP 1 (Cbc)" << std::endl
        << std::endl;

    MilpCbcOutput output(instance, parameters.info);

    VertexId n = instance.number_of_vertices();
    EdgeId m = instance.number_of_edges();

    // Variables
    int number_of_columns = n;
    std::vector<double> colum_lower_bounds(number_of_columns, 0);
    std::vector<double> colum_upper_bounds(number_of_columns, 1);

    // Objective
    std::vector<double> objective(number_of_columns);
    for (VertexId j = 0; j < n; ++j)
        objective[j] = instance.vertex(j).weight;

    // Constraints
    int number_of_rows = 0; // will be increased each time we add a constraint
    std::vector<CoinBigIndex> row_starts;
    std::vector<int> number_of_elements_in_rows;
    std::vector<int> element_columns;
    std::vector<double> elements;
    std::vector<double> row_lower_bounds;
    std::vector<double> row_upper_bounds;

    for (EdgeId e = 0; e < m; ++e) {
        row_starts.push_back(elements.size());
        number_of_elements_in_rows.push_back(0);
        number_of_rows++;
        element_columns.push_back(instance.edge(e).v1);
        element_columns.push_back(instance.edge(e).v2);
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
    for (VertexId j = 0; j < n; ++j)
        solver1.setInteger(j);

    // Pass data and solver to CbcModel
    CbcModel model(solver1);

    // Maximize.
    model.setObjSense(-1);

    // Callback
    SolHandler sh(instance, parameters, output);
    model.passInEventHandler(&sh);

    // Reduce printout
    model.setLogLevel(0);
    model.solver()->setHintParam(OsiDoReducePrint, true, OsiHintTry);

    // Set time limit
    model.setMaximumSeconds(parameters.info.remaining_time());

    // Do complete search
    model.branchAndBound();

    if (model.isProvenInfeasible()) {
        throw std::logic_error("");
    } else if (model.isProvenOptimal()) {
        if (!output.solution.feasible()
                || output.solution.weight() < -model.getObjValue()) {
            const double *solution_cbc = model.solver()->getColSolution();
            Solution solution(instance);
            for (VertexId j = 0; j < n; ++j)
                if (solution_cbc[j] > 0.5)
                    solution.add(j);
            output.update_solution(
                    solution,
                    std::stringstream(""),
                    parameters.info);
        }
        output.update_upper_bound(
                output.solution.weight(),
                std::stringstream(""),
                parameters.info);
    } else if (model.bestSolution() != NULL) {
        if (!output.solution.feasible()
                || output.solution.weight() < -model.getObjValue()) {
            const double *solution_cbc = model.solver()->getColSolution();
            Solution solution(instance);
            for (VertexId j = 0; j < n; ++j)
                if (solution_cbc[j] > 0.5)
                    solution.add(j);
            output.update_solution(
                    solution,
                    std::stringstream(""),
                    parameters.info);
        }
        Weight ub = -model.getBestPossibleObjValue();
        output.update_upper_bound(ub, std::stringstream(""), parameters.info);
    } else {
        Weight ub = -model.getBestPossibleObjValue();
        output.update_upper_bound(ub, std::stringstream(""), parameters.info);
    }

    return output.algorithm_end(parameters.info);
}

#endif

