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
        for (VertexId vertex_id = 0;
                vertex_id < instance_.number_of_vertices();
                ++vertex_id) {
            if (solution_cbc[vertex_id] > 0.5)
                solution.add(vertex_id);
        }
        output_.update_solution(
                solution,
                std::stringstream(""),
                parameters_.info);
    }

    return noAction;
}

////////////////////////////////////////////////////////////////////////////////
/////////////////////////// Model 1, |E| constraints ///////////////////////////
////////////////////////////////////////////////////////////////////////////////

MilpCbcOutput stablesolver::milp_1_cbc(
        const Instance& original_instance,
        MilpCbcOptionalParameters parameters)
{
    init_display(original_instance, parameters.info);
    parameters.info.os()
        << "Algorithm" << std::endl
        << "---------" << std::endl
        << "MILP 1 (Cbc)" << std::endl
        << std::endl;

    // Reduction.
    std::unique_ptr<Instance> reduced_instance = nullptr;
    if (parameters.reduction_parameters.reduce) {
        reduced_instance = std::unique_ptr<Instance>(
                new Instance(
                    original_instance.reduce(
                        parameters.reduction_parameters)));
        parameters.info.os()
            << "Reduced instance" << std::endl
            << "----------------" << std::endl;
        reduced_instance->print(parameters.info.os(), parameters.info.verbosity_level());
        parameters.info.os() << std::endl;
    }
    const Instance& instance = (reduced_instance == nullptr)? original_instance: *reduced_instance;

    MilpCbcOutput output(original_instance, parameters.info);

    // Update upper bound from reduction.
    if (reduced_instance != nullptr) {
        output.update_upper_bound(
                reduced_instance->total_weight()
                + reduced_instance->unreduction_info().extra_weight,
                std::stringstream("reduction"),
                parameters.info);
    }

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
            for (VertexId vertex_id = 0;
                    vertex_id < instance.number_of_vertices();
                    ++vertex_id) {
                if (solution_cbc[vertex_id] > 0.5)
                    solution.add(vertex_id);
            }
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
            for (VertexId vertex_id = 0;
                    vertex_id < instance.number_of_vertices();
                    ++vertex_id) {
                if (solution_cbc[vertex_id] > 0.5)
                    solution.add(vertex_id);
            }
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

