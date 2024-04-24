#include "stablesolver/clique/algorithms/milp_cplex.hpp"

#include "stablesolver/clique/algorithm_formatter.hpp"

#include <ilcplex/ilocplex.h>

using namespace stablesolver::clique;

ILOSTLBEGIN

ILOMIPINFOCALLBACK5(loggingCallback1,
                    const Instance&, instance,
                    const MilpCplexParameters&, parameters,
                    Output&, output,
                    AlgorithmFormatter&, algorithm_formatter,
                    IloNumVarArray&, x)
{
    VertexId ub = getBestObjValue();
    algorithm_formatter.update_bound(ub, "");

    if (!hasIncumbent())
        return;

    if (output.solution.weight() < getIncumbentObjValue() + 0.5) {
        Solution solution(instance);
        IloNumArray val(x.getEnv());
        getIncumbentValues(val, x);
        for (VertexId vertex_id = 0;
                vertex_id < instance.graph()->number_of_vertices();
                ++vertex_id) {
            if (val[vertex_id] > 0.5)
                solution.add(vertex_id);
        }
        algorithm_formatter.update_solution(solution, "");
    }
}

const Output stablesolver::clique::milp_cplex(
        const Instance& instance,
        const MilpCplexParameters& parameters)
{
    Output output(instance);
    AlgorithmFormatter algorithm_formatter(parameters, output);
    algorithm_formatter.start("MILP (CPLEX)");
    algorithm_formatter.print_header();

    const optimizationtools::AbstractGraph* graph = instance.graph();
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
        for (VertexId vertex_id = 0; vertex_id < n; ++vertex_id)
            expr += graph->weight(vertex_id) * x[vertex_id];
        IloObjective obj = IloMaximize(env, expr);
        model.add(obj);
    }

    // Constraints

    // Linking constraints between x and z.
    {
        IloExpr expr(env);
        for (VertexId vertex_id = 0; vertex_id < n; ++vertex_id)
            expr += x[vertex_id];
        model.add(z == expr);
    }

    // Conflict constraints.
    for (VertexId vertex_id = 0; vertex_id < n; ++vertex_id) {
        VertexId m = n - graph->degree(vertex_id) + 1;
        IloExpr expr(env);
        expr += z;
        for (auto it = graph->neighbors_begin(vertex_id);
                it != graph->neighbors_end(vertex_id);
                ++it) {
            expr -= x[*it];
        }
        model.add(expr <= m + 1 - m * x[vertex_id]);
    }

    IloCplex cplex(model);

    // Redirect standard output to log file
    std::ofstream logfile("cplex.log");
    cplex.setOut(logfile);

    cplex.setParam(IloCplex::Param::MIP::Tolerances::MIPGap, 0.0); // Fix precision issue
    cplex.setParam(IloCplex::Param::MIP::Strategy::File, 2); // Avoid running out of memory

    // Time limit
    if (parameters.timer.time_limit() != std::numeric_limits<double>::infinity())
        cplex.setParam(IloCplex::TiLim, parameters.timer.remaining_time());

    // Callback
    cplex.use(loggingCallback1(env, instance, parameters, output, algorithm_formatter, x));

    // Optimize
    cplex.solve();

    // Retrieve solution and bound.
    if (cplex.getStatus() == IloAlgorithm::Infeasible) {
    } else if (cplex.getStatus() == IloAlgorithm::Optimal) {
        if (output.solution.weight() < cplex.getObjValue() + 0.5) {
            Solution solution(instance);
            for (VertexId vertex_id = 0;
                    vertex_id < graph->number_of_vertices();
                    ++vertex_id) {
                if (cplex.getValue(x[vertex_id]) > 0.5)
                    solution.add(vertex_id);
            }
            algorithm_formatter.update_solution(solution, "");
        }
        algorithm_formatter.update_bound(
                output.solution.weight(),
                "");
    } else if (cplex.isPrimalFeasible()) {
        if (output.solution.weight() < cplex.getObjValue() + 0.5) {
            Solution solution(instance);
            for (VertexId vertex_id = 0;
                    vertex_id < graph->number_of_vertices();
                    ++vertex_id) {
                if (cplex.getValue(x[vertex_id]) > 0.5)
                    solution.add(vertex_id);
            }
            algorithm_formatter.update_solution(solution, "");
        }
        Weight ub = cplex.getBestObjValue();
        algorithm_formatter.update_bound(ub, "");
    } else {
        Weight ub = cplex.getBestObjValue();
        algorithm_formatter.update_bound(ub, "");
    }

    env.end();

    algorithm_formatter.end();
    return output;
}
