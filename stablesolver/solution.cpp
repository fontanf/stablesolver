#include "stablesolver/solution.hpp"

#include <iomanip>

using namespace stablesolver;

Solution::Solution(const Instance& instance):
    instance_(instance),
    vertices_(instance.vertex_number()),
    edges_(instance.edge_number(), 3),
    component_conflict_numbers_(instance.component_number(), 0),
    component_weights_(instance.component_number(), 0),
    penalties_(instance.edge_number(), 1)
{
    for (EdgeId e = 0; e < instance.edge_number(); ++e)
        edges_.set(e, 0);
}

Solution::Solution(const Instance& instance, std::string filepath):
    instance_(instance),
    vertices_(instance.vertex_number()),
    edges_(instance.edge_number(), 3),
    component_conflict_numbers_(instance.component_number(), 0),
    component_weights_(instance.component_number(), 0),
    penalties_(instance.edge_number(), 1)
{
    for (EdgeId e = 0; e < instance.edge_number(); ++e)
        edges_.set(e, 0);

    if (filepath.empty())
        return;
    std::ifstream file(filepath);
    if (!file.good()) {
        std::cerr << "\033[31m" << "ERROR, unable to open file \"" << filepath << "\"" << "\033[0m" << std::endl;
        return;
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
    edges_(solution.edges_),
    component_conflict_numbers_(solution.component_conflict_numbers_),
    component_weights_(solution.component_weights_),
    penalties_(solution.penalties_),
    weight_(solution.weight_),
    penalty_(solution.penalty_)
{ }

Solution& Solution::operator=(const Solution& solution)
{
    if (this != &solution) {
        assert(&instance_ == &solution.instance_);
        vertices_                   = solution.vertices_;
        edges_                      = solution.edges_;
        component_conflict_numbers_ = solution.component_conflict_numbers_;
        component_weights_          = solution.component_weights_;
        penalties_                  = solution.penalties_;
        weight_                     = solution.weight_;
        penalty_                    = solution.penalty_;
    }
    return *this;
}

void Solution::set_penalty(EdgeId e, Weight p)
{
    if (covers(e) == 2)
        penalty_ -= penalties_[e];
    penalties_[e] = p;
    if (covers(e) == 2)
        penalty_ += penalties_[e];
}

void Solution::increment_penalty(EdgeId e, Weight p)
{
    penalties_[e] += p;
    if (covers(e) == 2)
        penalty_ += p;
}

void Solution::write(std::string filepath)
{
    if (filepath.empty())
        return;
    std::ofstream cert(filepath);
    if (!cert.good()) {
        std::cerr << "\033[31m" << "ERROR, unable to open file \"" << filepath << "\"" << "\033[0m" << std::endl;
        assert(false);
        return;
    }

    //cert << vertex_number() << std::endl;
    for (VertexId v: vertices())
        cert << v << " ";
    cert.close();
}

std::ostream& stablesolver::operator<<(std::ostream& os, const Solution& solution)
{
    os << "n " << solution.vertex_number()
        << " w " << solution.weight()
        << std::endl;
    for (VertexId v: solution.vertices())
        os << v << " ";
    return os;
}

/*********************************** Output ***********************************/

Output::Output(const Instance& instance, Info& info):
    solution(instance),
    upper_bound(instance.total_weight() + 1)
{
    VER(info, std::left << std::setw(16) << "T (s)");
    VER(info, std::left << std::setw(16) << "LB");
    VER(info, std::left << std::setw(16) << "UB");
    VER(info, std::left << std::setw(16) << "GAP");
    VER(info, std::left << std::setw(16) << "GAP (%)");
    VER(info, "");
    VER(info, std::endl);
    print(info, std::stringstream(""));
}

void Output::print(Info& info, const std::stringstream& s) const
{
    double gap = (upper_bound == 0)?
        std::numeric_limits<double>::infinity():
        (double)(upper_bound - lower_bound()) / upper_bound * 100;
    double t = round(info.elapsed_time() * 10000) / 10000;

    VER(info, std::left << std::setw(16) << t);
    VER(info, std::left << std::setw(16) << lower_bound());
    VER(info, std::left << std::setw(16) << upper_bound);
    VER(info, std::left << std::setw(16) << upper_bound - lower_bound());
    VER(info, std::left << std::setw(16) << gap);
    VER(info, s.str() << std::endl);

    if (!info.output->onlywriteattheend)
        info.write_ini();
}

void Output::update_solution(
        const Solution& solution_new,
        ComponentId c,
        const std::stringstream& s,
        Info& info)
{
    info.output->mutex_sol.lock();

    bool ok = false;
    if (c == -1) {
        if (solution_new.feasible() && (!solution.feasible() || solution.weight() < solution_new.weight()))
            ok = true;
    } else {
        if (solution_new.feasible(c) && solution.weight(c) < solution_new.weight(c))
            ok = true;
    }

    if (ok) {
        if (c == -1) {
            for (VertexId v = 0; v < solution.instance().vertex_number(); ++v) {
                if (solution.contains(v) && !solution_new.contains(v)) {
                    solution.remove(v);
                } else if (!solution.contains(v) && solution_new.contains(v)) {
                    solution.add(v);
                }
            }
        } else {
            for (VertexId v: solution.instance().component(c).vertices) {
                if (solution.contains(v) && !solution_new.contains(v)) {
                    solution.remove(v);
                } else if (!solution.contains(v) && solution_new.contains(v)) {
                    solution.add(v);
                }
            }
        }
        print(info, s);

        info.output->sol_number++;
        double t = round(info.elapsed_time() * 10000) / 10000;
        std::string sol_str = "Solution" + std::to_string(info.output->sol_number);
        PUT(info, sol_str, "Value", solution.weight());
        PUT(info, sol_str, "Time", t);
        PUT(info, sol_str, "String", s.str());
        if (!info.output->onlywriteattheend) {
            info.write_ini();
            solution.write(info.output->certfile);
        }
    }

    info.output->mutex_sol.unlock();
}

void Output::update_upper_bound(Weight upper_bound_new, const std::stringstream& s, Info& info)
{
    if (upper_bound <= upper_bound_new)
        return;

    info.output->mutex_sol.lock();

    if (upper_bound > upper_bound_new) {
        upper_bound = upper_bound_new;
        print(info, s);

        info.output->bnd_number++;
        double t = round(info.elapsed_time() * 10000) / 10000;
        std::string sol_str = "Bound" + std::to_string(info.output->bnd_number);
        PUT(info, sol_str, "Bound", upper_bound);
        PUT(info, sol_str, "Time", t);
        PUT(info, sol_str, "String", s.str());
        if (!info.output->onlywriteattheend)
            solution.write(info.output->certfile);
    }

    info.output->mutex_sol.unlock();
}

Output& Output::algorithm_end(Info& info)
{
    double t = round(info.elapsed_time() * 10000) / 10000;
    double gap = (upper_bound == 0)?
        std::numeric_limits<double>::infinity():
        (double)(upper_bound - lower_bound()) / upper_bound * 100;
    PUT(info, "Solution", "Value", lower_bound());
    PUT(info, "Bound", "Value", upper_bound);
    PUT(info, "Solution", "Time", t);
    PUT(info, "Bound", "Time", t);
    VER(info, "---" << std::endl
            << "Value: " << lower_bound() << std::endl
            << "Vertex cover Value: " << solution.instance().total_weight() - lower_bound() << std::endl
            << "Bound: " << upper_bound << std::endl
            << "Gap: " << upper_bound - lower_bound() << std::endl
            << "Gap (%): " << gap << std::endl
            << "Time (s): " << t << std::endl
            );

    info.write_ini();
    solution.write(info.output->certfile);
    return *this;
}

Weight stablesolver::algorithm_end(Weight upper_bound, Info& info)
{
    double t = round(info.elapsed_time() * 10000) / 10000;
    PUT(info, "Bound", "Value", upper_bound);
    PUT(info, "Bound", "Time", t);
    VER(info, "---" << std::endl
            << "Bound: " << upper_bound << std::endl
            << "Time (s): " << t << std::endl
            );

    info.write_ini();
    return upper_bound;
}

