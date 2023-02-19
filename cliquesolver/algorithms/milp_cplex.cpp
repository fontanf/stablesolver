#if CPLEX_FOUND

#include "cliquesolver/algorithms/milp_cplex.hpp"

#include <ilcplex/ilocplex.h>

using namespace cliquesolver;

ILOSTLBEGIN

MilpCplexOutput& MilpCplexOutput::algorithm_end(
        optimizationtools::Info& info)
{
    //info.add_to_json("Algorithm", "Iterations", it);
    Output::algorithm_end(info);
    return *this;
}

ILOMIPINFOCALLBACK4(loggingCallback1,
                    const Instance&, instance,
                    MilpCplexOptionalParameters&, parameters,
                    MilpCplexOutput&, output,
                    IloNumVarArray&, x)
{
    VertexId ub = getBestObjValue();
    output.update_upper_bound(ub, std::stringstream(""), parameters.info);

    if (!hasIncumbent())
        return;

    if (output.solution.weight() < getIncumbentObjValue() + 0.5) {
        Solution solution(instance);
        IloNumArray val(x.getEnv());
        getIncumbentValues(val, x);
        for (VertexId v = 0; v < instance.graph()->number_of_vertices(); ++v)
            if (val[v] > 0.5)
                solution.add(v);
        output.update_solution(solution, std::stringstream(""), parameters.info);
    }
}

MilpCplexOutput cliquesolver::milp_cplex(
        const Instance& instance, MilpCplexOptionalParameters parameters)
{
    cliquesolver::init_display(instance, parameters.info);
    parameters.info.os()
        << "Algorithm" << std::endl
        << "---------" << std::endl
        << "MILP (CPLEX)" << std::endl
        << std::endl;

    const optimizationtools::AbstractGraph* graph = instance.graph();
    MilpCplexOutput output(instance, parameters.info);
    VertexId n = graph->number_of_vertices();

    IloEnv env;
    IloModel model(env);

    // Variables
    // x[v] == 1 iff vertex is chosen.
    IloNumVarArray x(env, n, 0, 1, ILOBOOL);
    // Auxiliary variable.
    IloNumVar z(env, 0, n, ILOINT);

    // Objective
    {
        IloExpr expr(env);
        for (VertexId v = 0; v < n; ++v)
            expr += graph->weight(v) * x[v];
        IloObjective obj = IloMaximize(env, expr);
        model.add(obj);
    }

    // Constraints

    // Linking constraints between x and z.
    {
        IloExpr expr(env);
        for (VertexId v = 0; v < n; ++v)
            expr += x[v];
        model.add(z == expr);
    }

    // Conflict constraints.
    for (VertexId v = 0; v < n; ++v) {
        VertexId m = n - graph->degree(v) + 1;
        IloExpr expr(env);
        expr += z;
        for (auto it = graph->neighbors_begin(v);
                it != graph->neighbors_end(v); ++it) {
            expr -= x[*it];
        }
        model.add(expr <= m + 1 - m * x[v]);
    }

    IloCplex cplex(model);

    // Redirect standard output to log file
    std::ofstream logfile("cplex.log");
    cplex.setOut(logfile);

    cplex.setParam(IloCplex::Param::MIP::Tolerances::MIPGap, 0.0); // Fix precision issue
    cplex.setParam(IloCplex::Param::MIP::Strategy::File, 2); // Avoid running out of memory

    // Time limit
    if (parameters.info.time_limit != std::numeric_limits<double>::infinity())
        cplex.setParam(IloCplex::TiLim, parameters.info.remaining_time());

    // Callback
    cplex.use(loggingCallback1(env, instance, parameters, output, x));

    // Optimize
    cplex.solve();

    // Retrieve solution and bound.
    if (cplex.getStatus() == IloAlgorithm::Infeasible) {
    } else if (cplex.getStatus() == IloAlgorithm::Optimal) {
        if (output.solution.weight() < cplex.getObjValue() + 0.5) {
            Solution solution(instance);
            for (VertexId v = 0; v < graph->number_of_vertices(); ++v)
                if (cplex.getValue(x[v]) > 0.5)
                    solution.add(v);
            output.update_solution(solution, std::stringstream(""), parameters.info);
        }
        output.update_upper_bound(output.solution.weight(), std::stringstream(""), parameters.info);
    } else if (cplex.isPrimalFeasible()) {
        if (output.solution.weight() < cplex.getObjValue() + 0.5) {
            Solution solution(instance);
            for (VertexId v = 0; v < graph->number_of_vertices(); ++v)
                if (cplex.getValue(x[v]) > 0.5)
                    solution.add(v);
            output.update_solution(solution, std::stringstream(""), parameters.info);
        }
        Weight ub = cplex.getBestObjValue();
        output.update_upper_bound(ub, std::stringstream(""), parameters.info);
    } else {
        Weight ub = cplex.getBestObjValue();
        output.update_upper_bound(ub, std::stringstream(""), parameters.info);
    }

    env.end();

    return output.algorithm_end(parameters.info);
}

#endif

