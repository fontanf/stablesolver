#include "stablesolver/stable/solution.hpp"

#include "optimizationtools/utils/utils.hpp"

#include <iomanip>
#include <fstream>

using namespace stablesolver::stable;

Solution::Solution(const Instance& instance):
    instance_(&instance),
    vertices_(instance.number_of_vertices()),
    component_number_of_conflictss_(instance.number_of_components(), 0),
    component_weights_(instance.number_of_components(), 0)
{
}

Solution::Solution(
        const Instance& instance,
        const std::string& certificate_path):
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

std::ostream& Solution::format(
        std::ostream& os,
        int verbosity_level) const
{
    if (verbosity_level >= 1) {
        os
            << "Number of vertices:   " << optimizationtools::Ratio<VertexId>(number_of_vertices(), instance().number_of_vertices()) << std::endl
            << "Number of conflicts:  " << number_of_conflicts() << std::endl
            << "Feasible:             " << feasible() << std::endl
            << "Vertex cover weight:  " << instance().total_weight() - weight() << std::endl
            << "Weight:               " << weight() << std::endl
            ;
    }

    if (verbosity_level >= 2) {
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

nlohmann::json Solution::to_json() const
{
    return nlohmann::json {
        {"NumberOfVertices", number_of_vertices()},
        {"Feasible", feasible()},
        {"Weight", weight()}
    };
}

void Solution::write(
        const std::string& certificate_path) const
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
