#include "stablesolver/clique/solution.hpp"

#include <fstream>
#include <iomanip>

using namespace stablesolver::clique;

Solution::Solution(const Instance& instance):
    instance_(&instance),
    vertices_(instance.graph()->number_of_vertices()),
    neighbors_tmp_(instance.graph()->number_of_vertices())
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
            << "Number of vertices:   " << optimizationtools::Ratio<VertexId>(number_of_vertices(), instance().graph()->number_of_vertices()) << std::endl
            << "Penalty:              " << penalty() << std::endl
            << "Feasible:             " << feasible() << std::endl
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
                << std::setw(12) << instance().graph()->weight(vertex_id)
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
    for (VertexId v: vertices())
        file << v << " ";
    file.close();
}

std::ostream& stablesolver::clique::operator<<(
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
