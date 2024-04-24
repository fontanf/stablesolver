#include "stablesolver/stable/instance.hpp"
#include "stablesolver/stable/instance_builder.hpp"

#include "optimizationtools/containers/indexed_set.hpp"

#include <iomanip>
#include <ostream>

using namespace stablesolver::stable;

const Instance Instance::complementary()
{
    InstanceBuilder instance_builder;
    instance_builder.add_vertices(number_of_vertices());
    optimizationtools::IndexedSet neighbors(number_of_vertices());

    for (VertexId vertex_id = 0;
            vertex_id < number_of_vertices();
            ++vertex_id) {
        instance_builder.set_weight(vertex_id, vertex(vertex_id).weight);
        neighbors.clear();
        neighbors.add(vertex_id);
        for (const auto& edge: vertex(vertex_id).edges)
            neighbors.add(edge.vertex_id);
        for (auto it = neighbors.out_begin(); it != neighbors.out_end(); ++it)
            if (*it > vertex_id)
                instance_builder.add_edge(vertex_id, *it);
    }

    return instance_builder.build();
}

std::ostream& Instance::format(
        std::ostream& os,
        int verbosity_level) const
{
    if (verbosity_level >= 1) {
        double density = (double)number_of_edges() * 2
            / number_of_vertices()
            / number_of_vertices();
        os
            << "Number of vertices:              " << number_of_vertices() << std::endl
            << "Number of edges:                 " << number_of_edges() << std::endl
            << "Density:                         " << density << std::endl
            << "Average degree:                  " << (double)number_of_edges() * 2 / number_of_vertices() << std::endl
            << "Highest degree:                  " << highest_degree() << std::endl
            << "Total weight:                    " << total_weight() << std::endl
            << "Number of connected components:  " << number_of_components() << std::endl
            ;
    }

    if (verbosity_level >= 2) {
        os << std::endl
            << std::setw(12) << "VertexId"
            << std::setw(12) << "Weight"
            << std::setw(12) << "Degree"
            << std::endl
            << std::setw(12) << "--------"
            << std::setw(12) << "------"
            << std::setw(12) << "------"
            << std::endl;
        for (VertexId vertex_id = 0;
                vertex_id < number_of_vertices();
                ++vertex_id) {
            const Vertex& vertex = this->vertex(vertex_id);
            os
                << std::setw(12) << vertex_id
                << std::setw(12) << vertex.weight
                << std::setw(12) << vertex.edges.size()
                << std::endl;
        }
    }

    if (verbosity_level >= 3) {
        os << std::endl
            << std::setw(12) << "Edge"
            << std::setw(12) << "Vertex 1"
            << std::setw(12) << "Vertex 2"
            << std::endl
            << std::setw(12) << "----"
            << std::setw(12) << "--------"
            << std::setw(12) << "--------"
            << std::endl;
        for (EdgeId edge_id = 0; edge_id < number_of_edges(); ++edge_id) {
            const Edge& edge = this->edge(edge_id);
            os
                << std::setw(12) << edge_id
                << std::setw(12) << edge.vertex_id_1
                << std::setw(12) << edge.vertex_id_2
                << std::endl;
        }
    }

    return os;
}
