#include "cliquesolver/solution.hpp"

#include <iomanip>

using namespace cliquesolver;

Solution::Solution(const Instance& instance):
    instance_(instance),
    vertices_(instance.graph()->number_of_vertices()),
    neighbors_tmp_(instance.graph()->number_of_vertices())
{
}

Solution::Solution(const Instance& instance, std::string certificate_path):
    instance_(instance),
    vertices_(instance.graph()->number_of_vertices()),
    neighbors_tmp_(instance.graph()->number_of_vertices())
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

Solution::Solution(const Solution& solution):
    instance_(solution.instance_),
    vertices_(solution.vertices_),
    weight_(solution.weight_),
    penalty_(solution.penalty_),
    neighbors_tmp_(solution.neighbors_tmp_)
{ }

Solution& Solution::operator=(const Solution& solution)
{
    if (this != &solution) {
        assert(&instance_ == &solution.instance_);
        vertices_      = solution.vertices_;
        weight_        = solution.weight_;
        penalty_       = solution.penalty_;
        neighbors_tmp_ = solution.neighbors_tmp_;
    }
    return *this;
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
    for (VertexId v: vertices())
        file << v << " ";
    file.close();
}

std::ostream& cliquesolver::operator<<(
        std::ostream& os,
        const Solution& solution)
{
    os << "n " << solution.number_of_vertices()
        << " w " << solution.weight()
        << std::endl;
    for (VertexId v: solution.vertices())
        os << v << " ";
    return os;
}

////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// Output ////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

Output::Output(
        const Instance& instance,
        optimizationtools::Info& info):
    solution(instance),
    upper_bound(instance.graph()->total_weight() + 1)
{
    FFOT_VER(info,
               std::setw(12) << "T (s)"
            << std::setw(12) << "LB"
            << std::setw(12) << "UB"
            << std::setw(12) << "GAP"
            << std::setw(12) << "GAP (%)"
            << std::setw(24) << "Comment"
            << std::endl
            << std::setw(12) << "-----"
            << std::setw(12) << "--"
            << std::setw(12) << "--"
            << std::setw(12) << "---"
            << std::setw(12) << "-------"
            << std::setw(24) << "-------"
            << std::endl);
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

    FFOT_VER(info,
               std::setw(12) << std::fixed << std::setprecision(3) << t << std::defaultfloat << std::setprecision(precision)
            << std::setw(12) << lower_bound()
            << std::setw(12) << upper_bound
            << std::setw(12) << upper_bound - lower_bound()
            << std::setw(12) << std::fixed << std::setprecision(2) << gap << std::defaultfloat << std::setprecision(precision)
            << std::setw(24) << s.str()
            << std::endl);

    if (!info.output->only_write_at_the_end)
        info.write_json_output();
}

void Output::update_solution(
        const Solution& solution_new,
        const std::stringstream& s,
        optimizationtools::Info& info)
{
    info.output->mutex_solutions.lock();

    VertexId n = solution.instance().graph()->number_of_vertices();
    if (solution_new.feasible() && solution.weight() < solution_new.weight()) {
        for (VertexId v = 0; v < n; ++v) {
            if (solution.contains(v) && !solution_new.contains(v)) {
                solution.remove(v);
            } else if (!solution.contains(v) && solution_new.contains(v)) {
                solution.add(v);
            }
        }
        print(info, s);

        info.output->number_of_solutions++;
        double t = round(info.elapsed_time() * 10000) / 10000;
        std::string sol_str = "Solution" + std::to_string(info.output->number_of_solutions);
        FFOT_PUT(info, sol_str, "Value", solution.weight());
        FFOT_PUT(info, sol_str, "Time", t);
        FFOT_PUT(info, sol_str, "String", s.str());
        if (!info.output->only_write_at_the_end) {
            info.write_json_output();
            solution.write(info.output->certificate_path);
        }
    }

    info.output->mutex_solutions.unlock();
}

void Output::update_upper_bound(
        Weight upper_bound_new,
        const std::stringstream& s,
        optimizationtools::Info& info)
{
    if (upper_bound <= upper_bound_new)
        return;

    info.output->mutex_solutions.lock();

    if (upper_bound > upper_bound_new) {
        upper_bound = upper_bound_new;
        print(info, s);

        info.output->number_of_bounds++;
        double t = round(info.elapsed_time() * 10000) / 10000;
        std::string sol_str = "Bound" + std::to_string(info.output->number_of_bounds);
        FFOT_PUT(info, sol_str, "Bound", upper_bound);
        FFOT_PUT(info, sol_str, "Time", t);
        FFOT_PUT(info, sol_str, "String", s.str());
        if (!info.output->only_write_at_the_end)
            solution.write(info.output->certificate_path);
    }

    info.output->mutex_solutions.unlock();
}

Output& Output::algorithm_end(
        optimizationtools::Info& info)
{
    double t = round(info.elapsed_time() * 10000) / 10000;
    double gap = (upper_bound == 0)?
        std::numeric_limits<double>::infinity():
        (double)(upper_bound - lower_bound()) / upper_bound * 100;
    FFOT_PUT(info, "Solution", "Value", lower_bound());
    FFOT_PUT(info, "Bound", "Value", upper_bound);
    FFOT_PUT(info, "Solution", "Time", t);
    FFOT_PUT(info, "Bound", "Time", t);
    FFOT_VER(info,
            std::endl
            << "Final statistics" << std::endl
            << "----------------" << std::endl
            << "Value:                 " << lower_bound() << std::endl
            << "Number of vertices:    " << solution.number_of_vertices() << std::endl
            << "Bound:                 " << upper_bound << std::endl
            << "Gap:                   " << upper_bound - lower_bound() << std::endl
            << "Gap (%):               " << gap << std::endl
            << "Time (s):              " << t << std::endl
            );

    info.write_json_output();
    solution.write(info.output->certificate_path);
    return *this;
}

Weight cliquesolver::algorithm_end(
        Weight upper_bound,
        optimizationtools::Info& info)
{
    double t = round(info.elapsed_time() * 10000) / 10000;
    FFOT_PUT(info, "Bound", "Value", upper_bound);
    FFOT_PUT(info, "Bound", "Time", t);
    FFOT_VER(info,
            std::endl
            << "Final statistics" << std::endl
            << "----------------" << std::endl
            << "Bound:                 " << upper_bound << std::endl
            << "Time (s):              " << t << std::endl
            );

    info.write_json_output();
    return upper_bound;
}

