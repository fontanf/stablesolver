#if CPLEX_FOUND

#include "stablesolver/algorithms/branchandcut_cplex.hpp"

#include <ilcplex/ilocplex.h>

using namespace stablesolver;

ILOSTLBEGIN

BranchAndCutCplexOutput& BranchAndCutCplexOutput::algorithm_end(Info& info)
{
    //PUT(info, "Algorithm", "Iterations", it);
    Output::algorithm_end(info);
    //VER(info, "Iterations: " << it << std::endl);
    return *this;
}

ILOMIPINFOCALLBACK4(loggingCallback,
                    const Instance&, instance,
                    BranchAndCutCplexOptionalParameters&, parameters,
                    BranchAndCutCplexOutput&, output,
                    IloNumVarArray&, x)
{
    VertexId ub = std::floor(getBestObjValue() + TOL);
    output.update_upper_bound(ub, std::stringstream(""), parameters.info);

    if (!hasIncumbent())
        return;

    if (output.solution.weight() < getIncumbentObjValue() + 0.5) {
        Solution solution(instance);
        IloNumArray val(x.getEnv());
        getIncumbentValues(val, x);
        for (VertexId v = 0; v < instance.vertex_number(); ++v)
            if (val[v] > 0.5)
                solution.add(v);
        output.update_solution(solution, std::stringstream(""), parameters.info);
    }
}

BranchAndCutCplexOutput stablesolver::branchandcut_cplex(
        const Instance& instance, BranchAndCutCplexOptionalParameters parameters)
{
    VER(parameters.info, "*** branchandcut_cplex ***" << std::endl);

    BranchAndCutCplexOutput output(instance, parameters.info);

    IloEnv env;
    IloModel model(env);

    // Variables
    // x[v] == 1 iff vertex is chosen.
    IloNumVarArray x(env, instance.vertex_number(), 0, 1, ILOBOOL);

    // Objective
    IloExpr expr(env);
    for (VertexId v = 0; v < instance.vertex_number(); ++v)
        expr += instance.vertex(v).weight * x[v];
    IloObjective obj = IloMaximize(env, expr);
    model.add(obj);

    // Constraints
    for (EdgeId e = 0; e < instance.edge_number(); ++e)
        model.add(x[instance.edge(e).v1] + x[instance.edge(e).v2] <= 1);

    IloCplex cplex(model);

    // Redirect standard output to log file
    std::ofstream logfile("cplex.log");
    cplex.setOut(logfile);

    cplex.setParam(IloCplex::Param::MIP::Tolerances::MIPGap, 0.0); // Fix precision issue
    cplex.setParam(IloCplex::Param::MIP::Strategy::File, 2); // Avoid running out of memory

    // Time limit
    if (parameters.info.timelimit != std::numeric_limits<double>::infinity())
        cplex.setParam(IloCplex::TiLim, parameters.info.remaining_time());

    // Callback
    cplex.use(loggingCallback(env, instance, parameters, output, x));

    // Optimize
    cplex.solve();

    // Retrieve solution and bound.
    if (cplex.getStatus() == IloAlgorithm::Infeasible) {
    } else if (cplex.getStatus() == IloAlgorithm::Optimal) {
        if (output.solution.weight() < cplex.getObjValue() + 0.5) {
            Solution solution(instance);
            for (VertexId v = 0; v < instance.vertex_number(); ++v)
                if (cplex.getValue(x[v]) > 0.5)
                    solution.add(v);
            output.update_solution(solution, std::stringstream(""), parameters.info);
        }
        output.update_upper_bound(output.solution.weight(), std::stringstream(""), parameters.info);
    } else if (cplex.isPrimalFeasible()) {
        if (output.solution.weight() < cplex.getObjValue() + 0.5) {
            Solution solution(instance);
            for (VertexId v = 0; v < instance.vertex_number(); ++v)
                if (cplex.getValue(x[v]) > 0.5)
                    solution.add(v);
            output.update_solution(solution, std::stringstream(""), parameters.info);
        }
        Weight ub = std::floor(cplex.getBestObjValue() + TOL);
        output.update_upper_bound(ub, std::stringstream(""), parameters.info);
    } else {
        Weight ub = std::floor(cplex.getBestObjValue() + TOL);
        output.update_upper_bound(ub, std::stringstream(""), parameters.info);
    }

    env.end();

    return output.algorithm_end(parameters.info);
}

#endif

