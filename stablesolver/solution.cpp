#include "stablesolver/solution.hpp"

#include "optimizationtools/utils/utils.hpp"

#include <iomanip>

using namespace stablesolver;

Solution::Solution(const Instance& instance):
    instance_(&instance),
    vertices_(instance.number_of_vertices()),
    component_number_of_conflictss_(instance.number_of_components(), 0),
    component_weights_(instance.number_of_components(), 0)
{
}

Solution::Solution(
        const Instance& instance,
        std::string certificate_path):
    Solution(instance)
{
    if (certificate_path.empty())
        return;
    std::ifstream file(certificate_path);
    if (!file.good()) {
        throw std::runtime_error(
                "Unable to open file \"" + certificate_path + "\".");
    }

    VertexId v;
    while (file.good()) {
        file >> v;
        add(v);
    }
}

void Solution::update(const Solution& solution)
{
    if (&instance() != &solution.instance()
            && &instance() != solution.instance().original_instance()) {
        throw std::runtime_error(
                "Cannot update a solution with a solution from a different instance.");
    }

    if (solution.instance().is_reduced()
            && solution.instance().original_instance() == &instance()) {
        for (VertexId vertex_id = 0;
                vertex_id < instance().number_of_vertices();
                ++vertex_id) {
            if (contains(vertex_id))
                remove(vertex_id);
        }
        for (VertexId vertex_id: solution.instance().unreduction_info().mandatory_vertices) {
            //std::cout << "mandatory " << vertex_id << std::endl;
            add(vertex_id);
        }
        for (VertexId vertex_id = 0;
                vertex_id < solution.instance().number_of_vertices();
                ++vertex_id) {
            if (solution.contains(vertex_id)) {
                for (VertexId vertex_id_2: solution.instance().unreduction_info().unreduction_operations[vertex_id].in) {
                    //std::cout << "+" << v << " => " << vertex_id_2 << std::endl;
                    add(vertex_id_2);
                }
            } else {
                for (VertexId vertex_id_2: solution.instance().unreduction_info().unreduction_operations[vertex_id].out) {
                    //std::cout << "-" << v << " => " << vertex_id_2 << std::endl;
                    add(vertex_id_2);
                }
            }
        }
        if (weight() != solution.weight() + solution.instance().unreduction_info().extra_weight) {
            throw std::runtime_error(
                    "Wrong weight after unreduction. Weight: "
                    + std::to_string(weight())
                    + "; reduced solution weight: "
                    + std::to_string(solution.weight())
                    + "; extra weight: "
                    + std::to_string(solution.instance().unreduction_info().extra_weight)
                    + ".");
        }
    } else {
        *this = solution;
    }
}

std::ostream& Solution::print(
        std::ostream& os,
        int verbose) const
{
    if (verbose >= 1) {
        os
            << "Number of vertices:   " << optimizationtools::Ratio<VertexId>(number_of_vertices(), instance().number_of_vertices()) << std::endl
            << "Number of conflicts:  " << number_of_conflicts() << std::endl
            << "Feasible:             " << feasible() << std::endl
            << "Vertex cover weight:  " << instance().total_weight() - weight() << std::endl
            << "Weight:               " << weight() << std::endl
            ;
    }

    if (verbose >= 2) {
        os << std::endl
            << std::setw(12) << "Vertex"
            << std::setw(12) << "Weight"
            << std::endl
            << std::setw(12) << "------"
            << std::setw(12) << "------"
            << std::endl;
        for (VertexId vertex_id: vertices_) {
            os
                << std::setw(12) << vertex_id
                << std::setw(12) << instance().vertex(vertex_id).weight
                << std::endl;
        }
    }

    return os;
}

void Solution::write(std::string certificate_path)
{
    if (certificate_path.empty())
        return;
    std::ofstream file(certificate_path);
    if (!file.good()) {
        throw std::runtime_error(
                "Unable to open file \"" + certificate_path + "\".");
    }

    //cert << number_of_vertices() << std::endl;
    for (VertexId vertex_id: vertices())
        file << vertex_id << " ";
    file.close();
}

bool Solution::is_strictly_better_than(const Solution& solution) const
{
    if (!feasible())
        return false;
    if (!solution.feasible())
        return true;
    Weight w1 = weight();
    if (instance().is_reduced())
        w1 += instance().unreduction_info().extra_weight;
    Weight w2 = solution.weight();
    //if (instance().is_reduced())
    //    w2 += solution.instance().unreduction_info().extra_weight;
    return w1 > w2;
}

std::ostream& stablesolver::operator<<(std::ostream& os, const Solution& solution)
{
    os << "n " << solution.number_of_vertices()
        << " w " << solution.weight()
        << std::endl;
    for (VertexId vertex_id: solution.vertices())
        os << vertex_id << " ";
    return os;
}

////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// Output ////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

Output::Output(
        const Instance& instance,
        optimizationtools::Info& info):
    solution(instance),
    bound(instance.total_weight())
{
    info.os()
        << std::setw(12) << "T (s)"
        << std::setw(16) << "LB"
        << std::setw(16) << "UB"
        << std::setw(16) << "GAP"
        << std::setw(12) << "GAP (%)"
        << std::setw(24) << "Comment"
        << std::endl
        << std::setw(12) << "-----"
        << std::setw(16) << "--"
        << std::setw(16) << "--"
        << std::setw(16) << "---"
        << std::setw(12) << "-------"
        << std::setw(24) << "-------"
        << std::endl;
    print(info, std::stringstream(""));
}

void Output::print(
        optimizationtools::Info& info,
        const std::stringstream& s) const
{
    std::string solution_value = optimizationtools::solution_value(
            optimizationtools::ObjectiveDirection::Maximize,
            solution.feasible(),
            solution.weight());
    double absolute_optimality_gap = optimizationtools::absolute_optimality_gap(
            optimizationtools::ObjectiveDirection::Maximize,
            solution.feasible(),
            solution.weight(),
            bound);
    double relative_optimality_gap = optimizationtools::relative_optimality_gap(
            optimizationtools::ObjectiveDirection::Maximize,
            solution.feasible(),
            solution.weight(),
            bound);
    double t = info.elapsed_time();
    std::streamsize precision = std::cout.precision();

    info.os()
        << std::setw(12) << std::fixed << std::setprecision(3) << t << std::defaultfloat << std::setprecision(precision)
        << std::setw(16) << solution_value
        << std::setw(16) << bound
        << std::setw(16) << absolute_optimality_gap
        << std::setw(12) << std::fixed << std::setprecision(2) << relative_optimality_gap * 100 << std::defaultfloat << std::setprecision(precision)
        << std::setw(24) << s.str()
        << std::endl;

    if (!info.output->only_write_at_the_end)
        info.write_json_output();
}

void Output::update_solution(
        const Solution& solution_new,
        const std::stringstream& s,
        optimizationtools::Info& info)
{
    info.lock();

    if (solution_new.is_strictly_better_than(solution)) {
        solution.update(solution_new);
        print(info, s);

        std::string solution_value = optimizationtools::solution_value(
                optimizationtools::ObjectiveDirection::Maximize,
                solution.feasible(),
                solution.weight());
        double t = info.elapsed_time();

        info.output->number_of_solutions++;
        std::string sol_str = "Solution" + std::to_string(info.output->number_of_solutions);
        info.add_to_json(sol_str, "Value", solution_value);
        info.add_to_json(sol_str, "Time", t);
        info.add_to_json(sol_str, "String", s.str());
        if (!info.output->only_write_at_the_end) {
            info.write_json_output();
            solution.write(info.output->certificate_path);
        }
    }

    info.unlock();
}

void Output::update_bound(
        Weight bound_new,
        const std::stringstream& s,
        optimizationtools::Info& info)
{
    if (bound <= bound_new)
        return;

    info.lock();

    if (bound > bound_new) {
        bound = bound_new;
        print(info, s);

        double t = info.elapsed_time();

        info.output->number_of_bounds++;
        std::string sol_str = "Bound" + std::to_string(info.output->number_of_bounds);
        info.add_to_json(sol_str, "Bound", bound);
        info.add_to_json(sol_str, "Time", t);
        info.add_to_json(sol_str, "String", s.str());
        if (!info.output->only_write_at_the_end)
            solution.write(info.output->certificate_path);
    }

    info.unlock();
}

Output& Output::algorithm_end(
        optimizationtools::Info& info)
{
    std::string solution_value = optimizationtools::solution_value(
            optimizationtools::ObjectiveDirection::Maximize,
            solution.feasible(),
            solution.weight());
    double absolute_optimality_gap = optimizationtools::absolute_optimality_gap(
            optimizationtools::ObjectiveDirection::Maximize,
            solution.feasible(),
            solution.weight(),
            bound);
    double relative_optimality_gap = optimizationtools::relative_optimality_gap(
            optimizationtools::ObjectiveDirection::Maximize,
            solution.feasible(),
            solution.weight(),
            bound);
    time = info.elapsed_time();

    info.add_to_json("Solution", "Value", solution_value);
    info.add_to_json("Bound", "Value", bound);
    info.add_to_json("Solution", "Time", time);
    info.add_to_json("Bound", "Time", time);
    info.os()
        << std::endl
        << "Final statistics" << std::endl
        << "----------------" << std::endl
        << "Value:                        " << solution_value << std::endl
        << "Bound:                        " << bound << std::endl
        << "Absolute optimality gap:      " << absolute_optimality_gap << std::endl
        << "Relative optimality gap (%):  " << relative_optimality_gap * 100 << std::endl
        << "Time (s):                     " << time << std::endl
        ;
    print_statistics(info);
    info.os() << std::endl
        << "Solution" << std::endl
        << "--------" << std::endl ;
    solution.print(info.os(), info.verbosity_level());

    info.write_json_output();
    solution.write(info.output->certificate_path);
    return *this;
}
