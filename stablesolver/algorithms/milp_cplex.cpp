#if CPLEX_FOUND

#include "stablesolver/algorithms/milp_cplex.hpp"

#include "cliquesolver/algorithms/greedy.hpp"

#include <ilcplex/ilocplex.h>

using namespace stablesolver;

ILOSTLBEGIN

MilpCplexOutput& MilpCplexOutput::algorithm_end(Info& info)
{
    //PUT(info, "Algorithm", "Iterations", it);
    Output::algorithm_end(info);
    //VER(info, "Iterations: " << it << std::endl);
    return *this;
}

ILOMIPINFOCALLBACK4(loggingCallback1,
                    const Instance&, instance,
                    MilpCplexOptionalParameters&, parameters,
                    MilpCplexOutput&, output,
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

/************************** Model 1, |E| constraints **************************/

MilpCplexOutput stablesolver::milp_1_cplex(
        const Instance& instance, MilpCplexOptionalParameters parameters)
{
    VER(parameters.info, "*** milp_1_cplex ***" << std::endl);

    MilpCplexOutput output(instance, parameters.info);

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
    cplex.use(loggingCallback1(env, instance, parameters, output, x));

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

/************************** Model 2, |V| constraints **************************/

MilpCplexOutput stablesolver::milp_2_cplex(
        const Instance& instance, MilpCplexOptionalParameters parameters)
{
    VER(parameters.info, "*** milp_2_cplex ***" << std::endl);

    MilpCplexOutput output(instance, parameters.info);

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
    for (VertexId v = 0; v < instance.vertex_number(); ++v) {
        IloExpr expr(env);
        expr += instance.degree(v) * x[v];
        for (const auto& edge: instance.vertex(v).edges)
            expr += x[edge.v];
        model.add(expr <= instance.degree(v));
    }

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
    cplex.use(loggingCallback1(env, instance, parameters, output, x));

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

/************************** Model 1, |E| constraints **************************/

MilpCplexOutput stablesolver::milp_3_cplex(
        const Instance& instance, MilpCplexOptionalParameters parameters)
{
    VER(parameters.info, "*** milp_3_cplex ***" << std::endl);

    MilpCplexOutput output(instance, parameters.info);

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
    optimizationtools::IndexedSet edge_set(instance.edge_number());
    optimizationtools::IndexedSet vertex_set_1(instance.vertex_number());
    optimizationtools::IndexedSet vertex_set_2(instance.vertex_number());
    std::vector<EdgeId> edge_indices(instance.edge_number(), 0);
    for (EdgeId e = 0; e < instance.edge_number(); ++e) {
        if (edge_set.contains(e))
            continue;
        // Compute vertices
        VertexId v1 = instance.edge(e).v1;
        VertexId v2 = instance.edge(e).v2;
        vertex_set_1.clear();
        vertex_set_2.clear();
        for (const auto& edge: instance.vertex(v1).edges)
            if (edge.v != v2)
                vertex_set_1.add(edge.v);
        for (const auto& edge: instance.vertex(v2).edges)
            if (vertex_set_1.contains(edge.v))
                vertex_set_2.add(edge.v);
        cliquesolver::Instance instance_clique(vertex_set_2.size());
        // Add edges
        for (auto it = vertex_set_2.begin(); it != vertex_set_2.end(); ++it) {
            for (const auto& edge: instance.vertex(*it).edges) {
                if (edge.v > *it && vertex_set_2.contains(edge.v)) {
                    edge_indices[instance_clique.edge_number()] = edge.e;
                    instance_clique.add_edge(vertex_set_2.position(*it), vertex_set_2.position(edge.v));
                }
            }
        }
        // Solve
        auto output_clique = cliquesolver::greedy_gwmin(instance_clique);
        // Build constraint
        IloExpr expr(env);
        expr += x[v1] + x[v2];
        for (const auto& edge: instance.vertex(v1).edges)
            if (vertex_set_2.contains(edge.v)
                    && output_clique.solution.contains(vertex_set_2.position(edge.v)))
                edge_set.add(edge.e);
        for (const auto& edge: instance.vertex(v2).edges)
            if (vertex_set_2.contains(edge.v)
                    && output_clique.solution.contains(vertex_set_2.position(edge.v)))
                edge_set.add(edge.e);
        for (VertexId v_clique: output_clique.solution.vertices()) {
            VertexId v_orig = *(vertex_set_2.begin() + v_clique);
            expr += x[v_orig];
            for (const auto& edge: instance_clique.vertex(v_clique).edges)
                if (output_clique.solution.contains(edge.v)
                        && edge.v > v_clique)
                    edge_set.add(edge_indices[edge.e]);
        }
        model.add(expr <= 1);
    }

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
    cplex.use(loggingCallback1(env, instance, parameters, output, x));

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

