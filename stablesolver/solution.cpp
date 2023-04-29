#include "stablesolver/solution.hpp"

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
    if (instance().reduced_instance() == solution.instance_) {
        for (VertexId vertex_id = 0;
                vertex_id < instance().number_of_vertices();
                ++vertex_id) {
            if (contains(vertex_id))
                remove(vertex_id);
        }
        for (VertexId vertex_id: instance().mandatory_vertices()) {
            //std::cout << "mandatory " << vertex_id << std::endl;
            add(vertex_id);
        }
        for (VertexId vertex_id = 0;
                vertex_id < instance().reduced_instance()->number_of_vertices();
                ++vertex_id) {
            if (solution.contains(vertex_id)) {
                for (VertexId vertex_id_2: instance().unreduction_operations(vertex_id).in) {
                    //std::cout << "+" << v << " => " << vertex_id_2 << std::endl;
                    add(vertex_id_2);
                }
            } else {
                for (VertexId vertex_id_2: instance().unreduction_operations(vertex_id).out) {
                    //std::cout << "-" << v << " => " << vertex_id_2 << std::endl;
                    add(vertex_id_2);
                }
            }
        }
        if (weight() != solution.weight() + instance().extra_weight()) {
            throw std::runtime_error(
                    "Wrong weight after unreduction. Weight: "
                    + std::to_string(weight())
                    + "; reduced solution weight: "
                    + std::to_string(solution.weight())
                    + "; extra weight: "
                    + std::to_string(instance().extra_weight())
                    + ".");
        }
    } else {
        *this = solution;
    }
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

bool Solution::is_striclty_better_than(const Solution& solution) const
{
    if (!feasible())
        return false;
    if (!solution.feasible())
        return true;
    if (instance_ == solution.instance_) {
        return weight() > solution.weight();
    } else if (instance_ == solution.instance().reduced_instance()) {
        return weight() + solution.instance().extra_weight() > solution.weight();
    } else if (instance().reduced_instance() == solution.instance_) {
        return weight() > solution.weight() + instance().extra_weight();
    } else {
        throw std::runtime_error(
                "Cannot compare solutions from different instances.");
    }
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

Output::Output(const Instance& instance, optimizationtools::Info& info):
    solution(instance),
    upper_bound(instance.total_weight())
{
    if (instance.reduced_instance() != nullptr)
        upper_bound = instance.extra_weight() + instance.reduced_instance()->total_weight();
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
    double gap = (upper_bound == 0)?
        std::numeric_limits<double>::infinity():
        (double)(upper_bound - lower_bound()) / upper_bound * 100;
    double t = info.elapsed_time();
    std::streamsize precision = std::cout.precision();

    info.os()
        << std::setw(12) << std::fixed << std::setprecision(3) << t << std::defaultfloat << std::setprecision(precision)
        << std::setw(16) << lower_bound()
        << std::setw(16) << upper_bound
        << std::setw(16) << upper_bound - lower_bound()
        << std::setw(12) << std::fixed << std::setprecision(2) << gap << std::defaultfloat << std::setprecision(precision)
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

    if (solution_new.is_striclty_better_than(solution)) {
        solution.update(solution_new);
        print(info, s);

        info.output->number_of_solutions++;
        double t = round(info.elapsed_time() * 10000) / 10000;
        std::string sol_str = "Solution" + std::to_string(info.output->number_of_solutions);
        info.add_to_json(sol_str, "Value", solution.weight());
        info.add_to_json(sol_str, "Time", t);
        info.add_to_json(sol_str, "String", s.str());
        if (!info.output->only_write_at_the_end) {
            info.write_json_output();
            solution.write(info.output->certificate_path);
        }
    }

    info.unlock();
}

void Output::update_upper_bound(
        Weight upper_bound_new,
        const std::stringstream& s,
        optimizationtools::Info& info)
{
    if (upper_bound <= upper_bound_new)
        return;

    info.lock();

    if (upper_bound > upper_bound_new) {
        upper_bound = upper_bound_new;
        print(info, s);

        info.output->number_of_bounds++;
        double t = round(info.elapsed_time() * 10000) / 10000;
        std::string sol_str = "Bound" + std::to_string(info.output->number_of_bounds);
        info.add_to_json(sol_str, "Bound", upper_bound);
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
    double t = round(info.elapsed_time() * 10000) / 10000;
    double gap = (upper_bound == 0)?
        std::numeric_limits<double>::infinity():
        (double)(upper_bound - lower_bound()) / upper_bound * 100;
    info.add_to_json("Solution", "Value", lower_bound());
    info.add_to_json("Bound", "Value", upper_bound);
    info.add_to_json("Solution", "Time", t);
    info.add_to_json("Bound", "Time", t);
    info.os()
        << std::endl
        << "Final statistics" << std::endl
        << "----------------" << std::endl
        << "Value:                 " << lower_bound() << std::endl
        << "Number of vertices:    " << solution.number_of_vertices() << std::endl
        << "Vertex cover Value:    " << solution.instance().total_weight() - lower_bound() << std::endl
        << "Bound:                 " << upper_bound << std::endl
        << "Gap:                   " << upper_bound - lower_bound() << std::endl
        << "Gap (%):               " << gap << std::endl
        << "Time (s):              " << t << std::endl
        ;

    info.write_json_output();
    solution.write(info.output->certificate_path);
    return *this;
}

Weight stablesolver::algorithm_end(
        Weight upper_bound,
        optimizationtools::Info& info)
{
    double t = round(info.elapsed_time() * 10000) / 10000;
    info.add_to_json("Bound", "Value", upper_bound);
    info.add_to_json("Bound", "Time", t);
    info.os()
        << std::endl
        << "Final statistics" << std::endl
        << "----------------" << std::endl
        << "Bound:                 " << upper_bound << std::endl
        << "Time (s):              " << t << std::endl
        ;

    info.write_json_output();
    return upper_bound;
}

