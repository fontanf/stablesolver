#if CPLEX_FOUND

#include "stablesolver/algorithms/milp_cplex.hpp"

#include "cliquesolver/algorithms/greedy.hpp"

#include <ilcplex/ilocplex.h>

using namespace stablesolver;

ILOSTLBEGIN

MilpCplexOutput& MilpCplexOutput::algorithm_end(
        optimizationtools::Info& info)
{
    //FFOT_PUT(info, "Algorithm", "Iterations", it);
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
        for (VertexId v = 0; v < instance.number_of_vertices(); ++v)
            if (val[v] > 0.5)
                solution.add(v);
        output.update_solution(solution, std::stringstream(""), parameters.info);
    }
}

////////////////////////////////////////////////////////////////////////////////
/////////////////////////// Model 1, |E| constraints ///////////////////////////
////////////////////////////////////////////////////////////////////////////////

MilpCplexOutput stablesolver::milp_1_cplex(
        const Instance& instance, MilpCplexOptionalParameters parameters)
{
    init_display(instance, parameters.info);
    parameters.info.os()
        << "Algorithm" << std::endl
        << "---------" << std::endl
        << "MILP 1 (CPLEX)" << std::endl
        << std::endl;

    MilpCplexOutput output(instance, parameters.info);

    IloEnv env;
    IloModel model(env);

    // Variables
    // x[v] == 1 iff vertex is chosen.
    IloNumVarArray x(env, instance.number_of_vertices(), 0, 1, ILOBOOL);

    // Objective
    IloExpr expr(env);
    for (VertexId vertex_id = 0;
            vertex_id < instance.number_of_vertices();
            ++vertex_id) {
        expr += instance.vertex(vertex_id).weight * x[vertex_id];
    }
    IloObjective obj = IloMaximize(env, expr);
    model.add(obj);

    // Constraints
    for (EdgeId edge_id = 0; edge_id < instance.number_of_edges(); ++edge_id) {
        model.add(x[
                instance.edge(edge_id).vertex_id_1]
                + x[instance.edge(edge_id).vertex_id_2] <= 1);
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
            for (VertexId vertex_id = 0;
                    vertex_id < instance.number_of_vertices();
                    ++vertex_id)
                if (cplex.getValue(x[vertex_id]) > 0.5)
                    solution.add(vertex_id);
            output.update_solution(
                    solution,
                    std::stringstream(""),
                    parameters.info);
        }
        output.update_upper_bound(
                output.solution.weight(),
                std::stringstream(""),
                parameters.info);
    } else if (cplex.isPrimalFeasible()) {
        if (output.solution.weight() < cplex.getObjValue() + 0.5) {
            Solution solution(instance);
            for (VertexId vertex_id = 0;
                    vertex_id < instance.number_of_vertices();
                    ++vertex_id) {
                if (cplex.getValue(x[vertex_id]) > 0.5)
                    solution.add(vertex_id);
            }
            output.update_solution(
                    solution,
                    std::stringstream(""),
                    parameters.info);
        }
        Weight ub = cplex.getBestObjValue();
        output.update_upper_bound(
                ub,
                std::stringstream(""),
                parameters.info);
    } else {
        Weight ub = cplex.getBestObjValue();
        output.update_upper_bound(
                ub,
                std::stringstream(""),
                parameters.info);
    }

    env.end();

    return output.algorithm_end(parameters.info);
}

////////////////////////////////////////////////////////////////////////////////
/////////////////////////// Model 2, |V| constraints ///////////////////////////
////////////////////////////////////////////////////////////////////////////////

MilpCplexOutput stablesolver::milp_2_cplex(
        const Instance& instance, MilpCplexOptionalParameters parameters)
{
    init_display(instance, parameters.info);
    parameters.info.os()
        << "Algorithm" << std::endl
        << "---------" << std::endl
        << "MILP 2 (CPLEX)" << std::endl
        << std::endl;

    MilpCplexOutput output(instance, parameters.info);

    IloEnv env;
    IloModel model(env);

    // Variables
    // x[v] == 1 iff vertex is chosen.
    IloNumVarArray x(env, instance.number_of_vertices(), 0, 1, ILOBOOL);

    // Objective
    IloExpr expr(env);
    for (VertexId vertex_id = 0;
            vertex_id < instance.number_of_vertices();
            ++vertex_id) {
        expr += instance.vertex(vertex_id).weight * x[vertex_id];
    }
    IloObjective obj = IloMaximize(env, expr);
    model.add(obj);

    // Constraints
    for (VertexId vertex_id = 0;
            vertex_id < instance.number_of_vertices();
            ++vertex_id) {
        IloExpr expr(env);
        expr += instance.degree(vertex_id) * x[vertex_id];
        for (const auto& edge: instance.vertex(vertex_id).edges)
            expr += x[edge.vertex_id];
        model.add(expr <= instance.degree(vertex_id));
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
            for (VertexId vertex_id = 0;
                    vertex_id < instance.number_of_vertices();
                    ++vertex_id)
                if (cplex.getValue(x[vertex_id]) > 0.5)
                    solution.add(vertex_id);
            output.update_solution(
                    solution,
                    std::stringstream(""),
                    parameters.info);
        }
        output.update_upper_bound(
                output.solution.weight(),
                std::stringstream(""),
                parameters.info);
    } else if (cplex.isPrimalFeasible()) {
        if (output.solution.weight() < cplex.getObjValue() + 0.5) {
            Solution solution(instance);
            for (VertexId vertex_id = 0;
                    vertex_id < instance.number_of_vertices();
                    ++vertex_id) {
                if (cplex.getValue(x[vertex_id]) > 0.5)
                    solution.add(vertex_id);
            }
            output.update_solution(
                    solution,
                    std::stringstream(""),
                    parameters.info);
        }
        Weight ub = cplex.getBestObjValue();
        output.update_upper_bound(
                ub,
                std::stringstream(""),
                parameters.info);
    } else {
        Weight ub = cplex.getBestObjValue();
        output.update_upper_bound(
                ub,
                std::stringstream(""),
                parameters.info);
    }

    env.end();

    return output.algorithm_end(parameters.info);
}

////////////////////////////////////////////////////////////////////////////////
/////////////////////////// Model 1, |E| constraints ///////////////////////////
////////////////////////////////////////////////////////////////////////////////

MilpCplexOutput stablesolver::milp_3_cplex(
        const Instance& instance, MilpCplexOptionalParameters parameters)
{
    init_display(instance, parameters.info);
    parameters.info.os()
        << "Algorithm" << std::endl
        << "---------" << std::endl
        << "MILP 3 (CPLEX)" << std::endl
        << std::endl;

    MilpCplexOutput output(instance, parameters.info);

    IloEnv env;
    IloModel model(env);

    // Variables
    // x[v] == 1 iff vertex is chosen.
    IloNumVarArray x(env, instance.number_of_vertices(), 0, 1, ILOBOOL);

    // Objective
    IloExpr expr(env);
    for (VertexId vertex_id = 0;
            vertex_id < instance.number_of_vertices();
            ++vertex_id) {
        expr += instance.vertex(vertex_id).weight * x[vertex_id];
    }
    IloObjective obj = IloMaximize(env, expr);
    model.add(obj);

    // Constraints
    optimizationtools::IndexedSet edge_set(instance.number_of_edges());
    optimizationtools::IndexedSet vertex_set_1(instance.number_of_vertices());
    optimizationtools::IndexedSet vertex_set_2(instance.number_of_vertices());
    std::vector<EdgeId> edge_indices(instance.number_of_edges(), 0);
    for (EdgeId e = 0; e < instance.number_of_edges(); ++e) {
        if (edge_set.contains(e))
            continue;
        // Compute vertices
        VertexId vertex_id_1 = instance.edge(e).vertex_id_1;
        VertexId vertex_id_2 = instance.edge(e).vertex_id_2;
        vertex_set_1.clear();
        vertex_set_2.clear();
        for (const auto& edge: instance.vertex(vertex_id_1).edges)
            if (edge.vertex_id != vertex_id_2)
                vertex_set_1.add(edge.vertex_id);
        for (const auto& edge: instance.vertex(vertex_id_2).edges)
            if (vertex_set_1.contains(edge.vertex_id))
                vertex_set_2.add(edge.vertex_id);
        cliquesolver::Instance instance_clique(vertex_set_2.size());
        // Add edges
        for (auto it = vertex_set_2.begin(); it != vertex_set_2.end(); ++it) {
            for (const auto& edge: instance.vertex(*it).edges) {
                if (edge.vertex_id > *it
                        && vertex_set_2.contains(edge.vertex_id)) {
                    edge_indices[instance_clique.adjacency_list_graph()->number_of_edges()] = edge.edge_id;
                    instance_clique.add_edge(
                            vertex_set_2.position(*it),
                            vertex_set_2.position(vertex_id_2));
                }
            }
        }
        // Solve
        auto output_clique = cliquesolver::greedy_gwmin(instance_clique);
        // Build constraint
        IloExpr expr(env);
        expr += x[vertex_id_1] + x[vertex_id_2];
        for (const auto& edge: instance.vertex(vertex_id_1).edges)
            if (vertex_set_2.contains(edge.vertex_id)
                    && output_clique.solution.contains(vertex_set_2.position(edge.vertex_id)))
                edge_set.add(edge.edge_id);
        for (const auto& edge: instance.vertex(vertex_id_2).edges)
            if (vertex_set_2.contains(edge.vertex_id)
                    && output_clique.solution.contains(vertex_set_2.position(edge.vertex_id)))
                edge_set.add(edge.edge_id);
        for (VertexId vertex_id_clique: output_clique.solution.vertices()) {
            VertexId vertex_id_orig = *(vertex_set_2.begin() + vertex_id_clique);
            expr += x[vertex_id_orig];
            for (const auto& edge: instance_clique.adjacency_list_graph()->edges(vertex_id_orig)) {
                if (output_clique.solution.contains(edge.vertex_id)
                        && edge.vertex_id > vertex_id_clique)
                    edge_set.add(edge_indices[edge.edge_id]);
            }
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
            for (VertexId vertex_id = 0;
                    vertex_id < instance.number_of_vertices();
                    ++vertex_id) {
                if (cplex.getValue(x[vertex_id]) > 0.5)
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
    } else if (cplex.isPrimalFeasible()) {
        if (output.solution.weight() < cplex.getObjValue() + 0.5) {
            Solution solution(instance);
            for (VertexId vertex_id = 0;
                    vertex_id < instance.number_of_vertices();
                    ++vertex_id) {
                if (cplex.getValue(x[vertex_id]) > 0.5)
                    solution.add(vertex_id);
            }
            output.update_solution(
                    solution,
                    std::stringstream(""),
                    parameters.info);
        }
        Weight ub = cplex.getBestObjValue();
        output.update_upper_bound(
                ub,
                std::stringstream(""),
                parameters.info);
    } else {
        Weight ub = cplex.getBestObjValue();
        output.update_upper_bound(
                ub,
                std::stringstream(""),
                parameters.info);
    }

    env.end();

    return output.algorithm_end(parameters.info);
}

#endif

