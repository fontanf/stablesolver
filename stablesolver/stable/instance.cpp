#include "stablesolver/stable/instance.hpp"
#include "stablesolver/stable/instance_builder.hpp"

#include "optimizationtools/containers/indexed_map.hpp"
#include "optimizationtools/containers/doubly_indexed_map.hpp"

#include <random>
#include <set>
#include <sstream>
#include <iomanip>
#include <thread>

using namespace stablesolver::stable;

Instance Instance::complementary()
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

std::ostream& Instance::print(
        std::ostream& os,
        int verbose) const
{
    if (verbose >= 1) {
        double density = (double)number_of_edges() * 2
            / number_of_vertices()
            / number_of_vertices();
        os
            << "Number of vertices:              " << number_of_vertices() << std::endl
            << "Number of edges:                 " << number_of_edges() << std::endl
            << "Density:                         " << density << std::endl
            << "Average degree:                  " << (double)number_of_edges() * 2 / number_of_vertices() << std::endl
            << "Maximum degree:                  " << maximum_degree() << std::endl
            << "Total weight:                    " << total_weight() << std::endl
            << "Number of connected components:  " << number_of_components() << std::endl
            ;
    }

    if (verbose >= 2) {
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

    if (verbose >= 3) {
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

bool Instance::reduce_pendant_vertices()
{
    optimizationtools::DoublyIndexedMap fixed_vertices(number_of_vertices(), 2);
    for (VertexId vertex_id = 0;
            vertex_id < number_of_vertices();
            ++vertex_id) {
        if (degree(vertex_id) != 1)
            continue;
        Weight weight = vertex(vertex_id).weight;
        VertexId vertex_id_1 = vertex(vertex_id).edges[0].vertex_id;
        if (vertex(vertex_id_1).weight > weight)
            continue;
        fixed_vertices.set(vertex_id, 1);
        fixed_vertices.set(vertex(vertex_id).edges[0].vertex_id, 0);
    }
    //std::cout << fixed_vertices.number_of_elements() << std::endl;

    if (fixed_vertices.number_of_elements() == 0)
        return false;

    UnreductionInfo new_unreduction_info;
    new_unreduction_info.original_instance = unreduction_info_.original_instance;

    // Update mandatory_vertices.
    new_unreduction_info.mandatory_vertices = unreduction_info_.mandatory_vertices;
    for (auto it = fixed_vertices.begin(1); it != fixed_vertices.end(1); ++it) {
        VertexId vertex_id = *it;
        for (VertexId orig_vertex_id: unreduction_info_.unreduction_operations[vertex_id].in) {
            new_unreduction_info.mandatory_vertices.push_back(orig_vertex_id);
        }
    }
    for (auto it = fixed_vertices.begin(0); it != fixed_vertices.end(0); ++it) {
        VertexId vertex_id = *it;
        for (VertexId orig_vertex_id: unreduction_info_.unreduction_operations[vertex_id].out) {
            new_unreduction_info.mandatory_vertices.push_back(orig_vertex_id);
        }
    }
    // Create new instance and compute unreduction_operations.
    VertexId new_number_of_vertices = number_of_vertices() - fixed_vertices.number_of_elements();
    InstanceBuilder new_instance_builder;
    new_instance_builder.add_vertices(new_number_of_vertices);
    new_unreduction_info.unreduction_operations = std::vector<UnreductionOperations>(new_number_of_vertices);
    // Add vertices.
    std::vector<VertexId> original2reduced(number_of_vertices(), -1);
    VertexId new_vertex_id = 0;
    for (auto it = fixed_vertices.out_begin(); it != fixed_vertices.out_end(); ++it) {
        VertexId vertex_id = *it;
        original2reduced[vertex_id] = new_vertex_id;
        new_instance_builder.set_weight(new_vertex_id, vertex(vertex_id).weight);
        new_unreduction_info.unreduction_operations[new_vertex_id]
            = unreduction_info_.unreduction_operations[vertex_id];
        new_vertex_id++;
    }
    // Add edges.
    for (EdgeId edge_id = 0; edge_id < number_of_edges(); ++edge_id) {
        VertexId vertex_id_1 = edge(edge_id).vertex_id_1;
        VertexId vertex_id_2 = edge(edge_id).vertex_id_2;
        VertexId new_vertex_id_1 = original2reduced[vertex_id_1];
        VertexId new_vertex_id_2 = original2reduced[vertex_id_2];
        if (new_vertex_id_1 != -1 && new_vertex_id_2 != -1) {
            new_instance_builder.add_edge(
                    new_vertex_id_1,
                    new_vertex_id_2,
                    0);
        }
    }

    *this = new_instance_builder.build();
    unreduction_info_ = new_unreduction_info;
    return true;
}

bool Instance::reduce_isolated_vertex_removal()
{
    //std::cout << "Isolated vertex removal..." << std::endl;
    optimizationtools::IndexedSet neighbors(number_of_vertices());
    optimizationtools::DoublyIndexedMap fixed_vertices(number_of_vertices(), 2);
    for (VertexId vertex_id = 0;
            vertex_id < number_of_vertices();
            ++vertex_id) {
        if (fixed_vertices.contains(vertex_id))
            continue;
        bool neighbors_clique = true;
        for (const auto& edge: vertex(vertex_id).edges) {
            if (fixed_vertices.contains(edge.vertex_id))
                continue;
            // Check if all neighbors of v are neighbors of edge.v.
            neighbors.clear();
            neighbors.add(edge.vertex_id);
            for (const auto& edge_2: vertex(edge.vertex_id).edges)
                if (!fixed_vertices.contains(edge_2.vertex_id))
                    neighbors.add(edge_2.vertex_id);
            for (const auto& edge: vertex(vertex_id).edges) {
                if (!fixed_vertices.contains(edge.vertex_id)) {
                    if (!neighbors.contains(edge.vertex_id)
                            || vertex(vertex_id).weight
                            < vertex(edge.vertex_id).weight) {
                        neighbors_clique = false;
                        break;
                    }
                }
            }
            if (!neighbors_clique)
                break;
        }
        if (neighbors_clique) {
            fixed_vertices.set(vertex_id, 1);
            for (const auto& edge: vertex(vertex_id).edges) {
                fixed_vertices.set(edge.vertex_id, 0);
            }
        }
    }
    //std::cout << fixed_vertices.number_of_elements() << std::endl;

    if (fixed_vertices.number_of_elements() == 0)
        return false;

    UnreductionInfo new_unreduction_info;
    new_unreduction_info.original_instance = unreduction_info_.original_instance;

    // Update mandatory_vertices.
    new_unreduction_info.mandatory_vertices = unreduction_info_.mandatory_vertices;
    for (auto it = fixed_vertices.begin(1); it != fixed_vertices.end(1); ++it) {
        VertexId vertex_id = *it;
        for (VertexId orig_vertex_id: unreduction_info_.unreduction_operations[vertex_id].in) {
            new_unreduction_info.mandatory_vertices.push_back(orig_vertex_id);
        }
    }
    for (auto it = fixed_vertices.begin(0); it != fixed_vertices.end(0); ++it) {
        VertexId vertex_id = *it;
        for (VertexId orig_vertex_id: unreduction_info_.unreduction_operations[vertex_id].out) {
            new_unreduction_info.mandatory_vertices.push_back(orig_vertex_id);
        }
    }
    // Create new instance and compute unreduction_operations.
    VertexId new_number_of_vertices = number_of_vertices() - fixed_vertices.number_of_elements();
    InstanceBuilder new_instance_builder;
    new_instance_builder.add_vertices(new_number_of_vertices);
    new_unreduction_info.unreduction_operations = std::vector<UnreductionOperations>(new_number_of_vertices);
    // Add vertices.
    std::vector<VertexId> original2reduced(number_of_vertices(), -1);
    VertexId new_vertex_id = 0;
    for (auto it = fixed_vertices.out_begin(); it != fixed_vertices.out_end(); ++it) {
        VertexId vertex_id = *it;
        original2reduced[vertex_id] = new_vertex_id;
        new_instance_builder.set_weight(new_vertex_id, vertex(vertex_id).weight);
        new_unreduction_info.unreduction_operations[new_vertex_id]
            = unreduction_info_.unreduction_operations[vertex_id];
        new_vertex_id++;
    }
    // Add edges.
    for (EdgeId edge_id = 0; edge_id < number_of_edges(); ++edge_id) {
        VertexId vertex_id_1 = edge(edge_id).vertex_id_1;
        VertexId vertex_id_2 = edge(edge_id).vertex_id_2;
        VertexId new_vertex_id_1 = original2reduced[vertex_id_1];
        VertexId new_vertex_id_2 = original2reduced[vertex_id_2];
        if (new_vertex_id_1 != -1 && new_vertex_id_2 != -1) {
            new_instance_builder.add_edge(new_vertex_id_1, new_vertex_id_2, 0);
        }
    }

    *this = new_instance_builder.build();
    unreduction_info_ = new_unreduction_info;
    return true;
}

bool Instance::reduce_vertex_folding()
{
    //std::cout << "Vertex folding..." << std::endl;
    optimizationtools::IndexedSet folded_vertices(number_of_vertices());
    std::vector<std::tuple<VertexId, VertexId, VertexId>> folded_vertices_list;
    for (VertexId vertex_id = 0;
            vertex_id < number_of_vertices();
            ++vertex_id) {
        if (degree(vertex_id) != 2)
            continue;
        VertexId vertex_id_1 = vertex(vertex_id).edges[0].vertex_id;
        VertexId vertex_id_2 = vertex(vertex_id).edges[1].vertex_id;
        if (folded_vertices.contains(vertex_id)
                || folded_vertices.contains(vertex_id_1)
                || folded_vertices.contains(vertex_id_2))
            continue;
        if (vertex(vertex_id).weight != vertex(vertex_id_1).weight
                || vertex(vertex_id).weight != vertex(vertex_id_2).weight)
            continue;
        // Check if there exists an edge (vertex_id_1, vertex_id_2).
        bool ok = true;
        for (const auto& edge: vertex(vertex_id_1).edges) {
            if (edge.vertex_id == vertex_id_2) {
                ok = false;
                break;
            }
        }
        if (!ok)
            continue;
        //std::cout << "v " << v << " vertex_id_1 " << vertex_id_1 << " vertex_id_2 " << vertex_id_2 << std::endl;
        folded_vertices.add(vertex_id);
        folded_vertices.add(vertex_id_1);
        folded_vertices.add(vertex_id_2);
        folded_vertices_list.push_back({vertex_id, vertex_id_1, vertex_id_2});
    }
    //std::cout << folded_vertices.number_of_elements() << std::endl;

    if (folded_vertices_list.empty())
        return false;

    UnreductionInfo new_unreduction_info;
    new_unreduction_info.original_instance = unreduction_info_.original_instance;

    // Update mandatory_vertices.
    new_unreduction_info.mandatory_vertices = unreduction_info_.mandatory_vertices;
    // Update instance and unreduction_operations.
    VertexId new_number_of_vertices = number_of_vertices() - folded_vertices.size() + folded_vertices_list.size();
    InstanceBuilder new_instance_builder;
    new_instance_builder.add_vertices(new_number_of_vertices);
    new_unreduction_info.unreduction_operations = std::vector<UnreductionOperations>(new_number_of_vertices);
    // Add vertices.
    std::vector<VertexId> original2reduced(number_of_vertices(), -1);
    VertexId new_vertex_id = 0;
    for (auto it = folded_vertices.out_begin(); it != folded_vertices.out_end(); ++it) {
        VertexId vertex_id = *it;
        original2reduced[vertex_id] = new_vertex_id;
        new_instance_builder.set_weight(new_vertex_id, vertex(vertex_id).weight);
        new_unreduction_info.unreduction_operations[new_vertex_id]
            = unreduction_info_.unreduction_operations[vertex_id];
        new_vertex_id++;
    }
    for (const auto& tuple: folded_vertices_list) {
        VertexId vertex_id = std::get<0>(tuple);
        VertexId vertex_id_1 = std::get<1>(tuple);
        VertexId vertex_id_2 = std::get<2>(tuple);
        if (vertex_id_1 == vertex_id_2) {
            throw std::runtime_error(
                    "Vertex "
                    + std::to_string(vertex_id)
                    + " has two times vertex "
                    + std::to_string(vertex_id_1)
                    + " as neighbor.");
        }
        original2reduced[vertex_id] = new_vertex_id;
        original2reduced[vertex_id_1] = new_vertex_id;
        original2reduced[vertex_id_2] = new_vertex_id;
        new_instance_builder.set_weight(new_vertex_id, vertex(vertex_id).weight);

        for (VertexId vertex_id_3: unreduction_info_.unreduction_operations[vertex_id].out)
            new_unreduction_info.unreduction_operations[new_vertex_id].in.push_back(vertex_id_3);
        for (VertexId vertex_id_3: unreduction_info_.unreduction_operations[vertex_id].in)
            new_unreduction_info.unreduction_operations[new_vertex_id].out.push_back(vertex_id_3);

        for (VertexId vertex_id_3: unreduction_info_.unreduction_operations[vertex_id_1].in)
            new_unreduction_info.unreduction_operations[new_vertex_id].in.push_back(vertex_id_3);
        for (VertexId vertex_id_3: unreduction_info_.unreduction_operations[vertex_id_1].out)
            new_unreduction_info.unreduction_operations[new_vertex_id].out.push_back(vertex_id_3);

        for (VertexId vertex_id_3: unreduction_info_.unreduction_operations[vertex_id_2].in)
            new_unreduction_info.unreduction_operations[new_vertex_id].in.push_back(vertex_id_3);
        for (VertexId vertex_id_3: unreduction_info_.unreduction_operations[vertex_id_2].out)
            new_unreduction_info.unreduction_operations[new_vertex_id].out.push_back(vertex_id_3);

        new_vertex_id++;
    }
    // Add edges.
    for (EdgeId e = 0; e < number_of_edges(); ++e) {
        VertexId vertex_id_1 = edge(e).vertex_id_1;
        VertexId vertex_id_2 = edge(e).vertex_id_2;
        if (folded_vertices.contains(vertex_id_1) || folded_vertices.contains(vertex_id_2))
            continue;
        VertexId new_vertex_id_1 = original2reduced[vertex_id_1];
        VertexId new_vertex_id_2 = original2reduced[vertex_id_2];
        new_instance_builder.add_edge(new_vertex_id_1, new_vertex_id_2, 0);
    }
    optimizationtools::IndexedSet neighbors_tmp(new_number_of_vertices);
    for (const auto& tuple: folded_vertices_list) {
        VertexId vertex_id = std::get<0>(tuple);
        VertexId vertex_id_1 = std::get<1>(tuple);
        VertexId vertex_id_2 = std::get<2>(tuple);
        VertexId new_vertex_id = original2reduced[vertex_id];

        neighbors_tmp.clear();
        for (const auto& edge: vertex(vertex_id_1).edges)
            if (edge.vertex_id != vertex_id)
                if (!folded_vertices.contains(edge.vertex_id)
                        || new_vertex_id < original2reduced[edge.vertex_id])
                    neighbors_tmp.add(original2reduced[edge.vertex_id]);
        for (const auto& edge: vertex(vertex_id_2).edges)
            if (edge.vertex_id != vertex_id)
                if (!folded_vertices.contains(edge.vertex_id)
                        || new_vertex_id < original2reduced[edge.vertex_id])
                    neighbors_tmp.add(original2reduced[edge.vertex_id]);
        for (VertexId vertex_id_3_new: neighbors_tmp)
            new_instance_builder.add_edge(new_vertex_id, vertex_id_3_new, 0);
    }

    *this = new_instance_builder.build();
    unreduction_info_ = new_unreduction_info;
    return true;
}

bool Instance::reduce_twin()
{
    //std::cout << "Vertex folding..." << std::endl;
    // 0: removed
    // 1: added
    // 2: folded
    optimizationtools::DoublyIndexedMap modified_vertices(number_of_vertices(), 3);
    optimizationtools::IndexedMap<VertexPos> twin_candidates(number_of_vertices(), 0);
    std::vector<std::tuple<VertexId, VertexId, VertexId, VertexId, VertexId>> folded_vertices_list;
    for (VertexId vertex_id = 0;
            vertex_id < number_of_vertices();
            ++vertex_id) {
        if (degree(vertex_id) != 3)
            continue;
        VertexId vertex_id_1 = vertex(vertex_id).edges[0].vertex_id;
        VertexId vertex_id_2 = vertex(vertex_id).edges[1].vertex_id;
        VertexId vertex_id_3 = vertex(vertex_id).edges[2].vertex_id;
        if (modified_vertices.contains(vertex_id)
                || modified_vertices.contains(vertex_id_1)
                || modified_vertices.contains(vertex_id_2)
                || modified_vertices.contains(vertex_id_3))
            continue;
        Weight weight = vertex(vertex_id).weight;
        if (vertex(vertex_id_1).weight != weight
                || vertex(vertex_id_2).weight != weight
                || vertex(vertex_id_3).weight != weight)
            continue;
        twin_candidates.clear();
        for (const auto& edge: vertex(vertex_id).edges) {
            for (const auto& edge_2: vertex(edge.vertex_id).edges) {
                if (degree(edge_2.vertex_id) != 3)
                    continue;
                if (edge_2.vertex_id == vertex_id)
                    continue;
                if (vertex(edge_2.vertex_id).weight != weight)
                    continue;
                if (modified_vertices.contains(edge_2.vertex_id))
                    continue;
                twin_candidates.set(edge_2.vertex_id, twin_candidates[edge_2.vertex_id] + 1);
            }
        }
        VertexId vertex_id_twin = -1;
        for (auto p: twin_candidates) {
            if (p.second == 3) {
                vertex_id_twin = p.first;
                break;
            }
        }
        if (vertex_id_twin == -1)
            continue;
        // Is there an edge inside vertex_id_1, vertex_id_2, vertex_id_3?
        bool has_edge = false;
        for (const auto& edge: vertex(vertex_id_1).edges)
            if (edge.vertex_id == vertex_id_2 || edge.vertex_id == vertex_id_3)
                has_edge = true;
        for (const auto& edge: vertex(vertex_id_2).edges)
            if (edge.vertex_id == vertex_id_3)
                has_edge = true;

        if (has_edge) {
            modified_vertices.set(vertex_id, 1);
            modified_vertices.set(vertex_id_twin, 1);
            modified_vertices.set(vertex_id_1, 0);
            modified_vertices.set(vertex_id_2, 0);
            modified_vertices.set(vertex_id_3, 0);
        } else {
            modified_vertices.set(vertex_id, 2);
            modified_vertices.set(vertex_id_twin, 2);
            modified_vertices.set(vertex_id_1, 2);
            modified_vertices.set(vertex_id_2, 2);
            modified_vertices.set(vertex_id_3, 2);
            folded_vertices_list.push_back({
                    vertex_id,
                    vertex_id_twin,
                    vertex_id_1,
                    vertex_id_2,
                    vertex_id_3});
        }
    }
    //std::cout << folded_vertices.number_of_elements() << std::endl;

    if (modified_vertices.number_of_elements() == 0)
        return false;

    UnreductionInfo new_unreduction_info;
    new_unreduction_info.original_instance = unreduction_info_.original_instance;

    // Update mandatory_vertices.
    new_unreduction_info.mandatory_vertices = unreduction_info_.mandatory_vertices;
    for (auto it = modified_vertices.begin(1); it != modified_vertices.end(1); ++it) {
        VertexId vertex_id = *it;
        for (VertexId orig_vertex_id: unreduction_info_.unreduction_operations[vertex_id].in) {
            new_unreduction_info.mandatory_vertices.push_back(orig_vertex_id);
        }
    }
    for (auto it = modified_vertices.begin(0); it != modified_vertices.end(0); ++it) {
        VertexId vertex_id = *it;
        for (VertexId orig_vertex_id: unreduction_info_.unreduction_operations[vertex_id].out) {
            new_unreduction_info.mandatory_vertices.push_back(orig_vertex_id);
        }
    }
    // Create new instance and compute unreduction_operations.
    VertexId new_number_of_vertices = number_of_vertices()
        - modified_vertices.number_of_elements(0)
        - modified_vertices.number_of_elements(1)
        - modified_vertices.number_of_elements(2)
        + modified_vertices.number_of_elements(2) / 5;
    InstanceBuilder new_instance_builder;
    new_instance_builder.add_vertices(new_number_of_vertices);
    new_unreduction_info.unreduction_operations = std::vector<UnreductionOperations>(new_number_of_vertices);
    // Add vertices.
    std::vector<VertexId> original2reduced(number_of_vertices(), -1);
    VertexId new_vertex_id = 0;
    for (auto it = modified_vertices.out_begin();
            it != modified_vertices.out_end();
            ++it) {
        VertexId vertex_id = *it;
        original2reduced[vertex_id] = new_vertex_id;
        new_instance_builder.set_weight(new_vertex_id, vertex(vertex_id).weight);
        new_unreduction_info.unreduction_operations[new_vertex_id]
            = unreduction_info_.unreduction_operations[vertex_id];
        new_vertex_id++;
    }
    for (const auto& tuple: folded_vertices_list) {
        VertexId vertex_id = std::get<0>(tuple);
        VertexId vertex_id_twin = std::get<1>(tuple);
        VertexId vertex_id_1 = std::get<2>(tuple);
        VertexId vertex_id_2 = std::get<3>(tuple);
        VertexId vertex_id_3 = std::get<4>(tuple);
        original2reduced[vertex_id] = new_vertex_id;
        original2reduced[vertex_id_twin] = new_vertex_id;
        original2reduced[vertex_id_1] = new_vertex_id;
        original2reduced[vertex_id_2] = new_vertex_id;
        original2reduced[vertex_id_3] = new_vertex_id;
        new_instance_builder.set_weight(new_vertex_id, vertex(vertex_id).weight);

        for (VertexId vertex_id_tmp: unreduction_info_.unreduction_operations[vertex_id].out)
            new_unreduction_info.unreduction_operations[new_vertex_id].in.push_back(vertex_id_tmp);
        for (VertexId vertex_id_tmp: unreduction_info_.unreduction_operations[vertex_id].in)
            new_unreduction_info.unreduction_operations[new_vertex_id].out.push_back(vertex_id_tmp);

        for (VertexId vertex_id_tmp: unreduction_info_.unreduction_operations[vertex_id_twin].out)
            new_unreduction_info.unreduction_operations[new_vertex_id].in.push_back(vertex_id_tmp);
        for (VertexId vertex_id_tmp: unreduction_info_.unreduction_operations[vertex_id_twin].in)
            new_unreduction_info.unreduction_operations[new_vertex_id].out.push_back(vertex_id_tmp);

        for (VertexId vertex_id_tmp: unreduction_info_.unreduction_operations[vertex_id_1].in)
            new_unreduction_info.unreduction_operations[new_vertex_id].in.push_back(vertex_id_tmp);
        for (VertexId vertex_id_tmp: unreduction_info_.unreduction_operations[vertex_id_1].out)
            new_unreduction_info.unreduction_operations[new_vertex_id].out.push_back(vertex_id_tmp);

        for (VertexId vertex_id_tmp: unreduction_info_.unreduction_operations[vertex_id_2].in)
            new_unreduction_info.unreduction_operations[new_vertex_id].in.push_back(vertex_id_tmp);
        for (VertexId vertex_id_tmp: unreduction_info_.unreduction_operations[vertex_id_2].out)
            new_unreduction_info.unreduction_operations[new_vertex_id].out.push_back(vertex_id_tmp);

        for (VertexId vertex_id_tmp: unreduction_info_.unreduction_operations[vertex_id_3].in)
            new_unreduction_info.unreduction_operations[new_vertex_id].in.push_back(vertex_id_tmp);
        for (VertexId vertex_id_tmp: unreduction_info_.unreduction_operations[vertex_id_3].out)
            new_unreduction_info.unreduction_operations[new_vertex_id].out.push_back(vertex_id_tmp);

        new_vertex_id++;
    }
    // Add edges.
    for (EdgeId edge_id = 0; edge_id < number_of_edges(); ++edge_id) {
        VertexId vertex_id_1 = edge(edge_id).vertex_id_1;
        VertexId vertex_id_2 = edge(edge_id).vertex_id_2;
        if (modified_vertices.contains(vertex_id_1) || modified_vertices.contains(vertex_id_2))
            continue;
        VertexId new_vertex_id_1 = original2reduced[vertex_id_1];
        VertexId new_vertex_id_2 = original2reduced[vertex_id_2];
        new_instance_builder.add_edge(new_vertex_id_1, new_vertex_id_2, 0);
    }
    optimizationtools::IndexedSet neighbors_tmp(new_number_of_vertices);
    for (const auto& tuple: folded_vertices_list) {
        VertexId vertex_id = std::get<0>(tuple);
        VertexId vertex_id_twin = std::get<1>(tuple);
        VertexId vertex_id_1 = std::get<2>(tuple);
        VertexId vertex_id_2 = std::get<3>(tuple);
        VertexId vertex_id_3 = std::get<4>(tuple);
        VertexId new_vertex_id = original2reduced[vertex_id];

        neighbors_tmp.clear();
        for (const auto& edge: vertex(vertex_id_1).edges)
            if (edge.vertex_id != vertex_id
                    && edge.vertex_id != vertex_id_twin)
                if (!modified_vertices.contains(edge.vertex_id)
                        || new_vertex_id < original2reduced[edge.vertex_id])
                    neighbors_tmp.add(original2reduced[edge.vertex_id]);
        for (const auto& edge: vertex(vertex_id_2).edges)
            if (edge.vertex_id != vertex_id
                    && edge.vertex_id != vertex_id_twin)
                if (!modified_vertices.contains(edge.vertex_id)
                        || new_vertex_id < original2reduced[edge.vertex_id])
                    neighbors_tmp.add(original2reduced[edge.vertex_id]);
        for (const auto& edge: vertex(vertex_id_3).edges)
            if (edge.vertex_id != vertex_id
                    && edge.vertex_id != vertex_id_twin)
                if (!modified_vertices.contains(edge.vertex_id)
                        || new_vertex_id < original2reduced[edge.vertex_id])
                    neighbors_tmp.add(original2reduced[edge.vertex_id]);
        for (VertexId vertex_id_tmp: neighbors_tmp)
            new_instance_builder.add_edge(new_vertex_id, vertex_id_tmp, 0);
    }

    *this = new_instance_builder.build();
    unreduction_info_ = new_unreduction_info;
    return true;
}

bool Instance::reduce_domination()
{
    //std::cout << "Vertex folding..." << std::endl;
    optimizationtools::IndexedSet removed_vertices(number_of_vertices());
    optimizationtools::IndexedSet neighbors(number_of_vertices());
    for (VertexId vertex_id = 0;
            vertex_id < number_of_vertices();
            ++vertex_id) {
        Weight weight = vertex(vertex_id).weight;
        neighbors.clear();
        for (const auto& edge: vertex(vertex_id).edges)
            neighbors.add(edge.vertex_id);
        bool can_be_removed = false;
        for (const auto& edge: vertex(vertex_id).edges) {
            if (vertex(edge.vertex_id).weight < weight)
                continue;
            if (removed_vertices.contains(edge.vertex_id))
                continue;
            bool dominates = true;
            for (const auto& edge_2: vertex(edge.vertex_id).edges) {
                if (edge_2.vertex_id != vertex_id
                        && !neighbors.contains(edge_2.vertex_id)) {
                    dominates = false;
                    break;
                }
            }
            if (dominates) {
                can_be_removed = true;
                break;
            }
        }
        if (can_be_removed)
            removed_vertices.add(vertex_id);
    }
    //std::cout << folded_vertices.number_of_elements() << std::endl;

    if (removed_vertices.size() == 0)
        return false;

    UnreductionInfo new_unreduction_info;
    new_unreduction_info.original_instance = unreduction_info_.original_instance;

    // Update mandatory_vertices.
    new_unreduction_info.mandatory_vertices = unreduction_info_.mandatory_vertices;
    for (VertexId vertex_id: removed_vertices) {
        for (VertexId orig_vertex_id: unreduction_info_.unreduction_operations[vertex_id].out) {
            new_unreduction_info.mandatory_vertices.push_back(orig_vertex_id);
        }
    }
    // Update instance and unreduction_operations.
    VertexId new_number_of_vertices = number_of_vertices() - removed_vertices.size();
    InstanceBuilder new_instance_builder;
    new_instance_builder.add_vertices(new_number_of_vertices);
    new_unreduction_info.unreduction_operations = std::vector<UnreductionOperations>(new_number_of_vertices);
    // Add vertices.
    std::vector<VertexId> original2reduced(number_of_vertices(), -1);
    VertexId new_vertex_id = 0;
    for (auto it = removed_vertices.out_begin(); it != removed_vertices.out_end(); ++it) {
        VertexId vertex_id = *it;
        original2reduced[vertex_id] = new_vertex_id;
        new_instance_builder.set_weight(new_vertex_id, vertex(vertex_id).weight);
        new_unreduction_info.unreduction_operations[new_vertex_id]
            = unreduction_info_.unreduction_operations[vertex_id];
        new_vertex_id++;
    }
    // Add edges.
    for (EdgeId edge_id = 0; edge_id < number_of_edges(); ++edge_id) {
        VertexId vertex_id_1 = edge(edge_id).vertex_id_1;
        VertexId vertex_id_2 = edge(edge_id).vertex_id_2;
        if (removed_vertices.contains(vertex_id_1) || removed_vertices.contains(vertex_id_2))
            continue;
        VertexId new_vertex_id_1 = original2reduced[vertex_id_1];
        VertexId new_vertex_id_2 = original2reduced[vertex_id_2];
        new_instance_builder.add_edge(new_vertex_id_1, new_vertex_id_2, 0);
    }

    *this = new_instance_builder.build();
    unreduction_info_ = new_unreduction_info;
    return true;
}

bool Instance::reduce_unconfined()
{
    //std::cout << "Vertex folding..." << std::endl;
    optimizationtools::IndexedSet removed_vertices(number_of_vertices());
    optimizationtools::IndexedSet s(number_of_vertices());
    optimizationtools::IndexedSet n_s(number_of_vertices());
    for (VertexId vertex_id = 0;
            vertex_id < number_of_vertices();
            ++vertex_id) {
        // Minimum weight in S.
        // The unconfined reduction rule remains true while the minimum weight
        // in S is greater or equal to the maximum weight in N(S).
        Weight ws_min = vertex(vertex_id).weight;
        Weight wns_max = 0;

        s.clear();
        n_s.clear();

        // Update S.
        s.add(vertex_id);
        // Update N(S).
        for (const auto& edge: vertex(vertex_id).edges) {
            n_s.add(edge.vertex_id);
            if (wns_max < vertex(edge.vertex_id).weight)
                wns_max = vertex(edge.vertex_id).weight;
        }
        if (ws_min < wns_max)
            continue;

        bool can_be_removed = false;
        for (;;) {
            VertexId u_best = -1;
            VertexPos n_u_minus_n_s_card_best = -1;
            VertexId w_best = -1;
            for (VertexId u: n_s) {
                if (removed_vertices.contains(u))
                    continue;
                VertexPos n_u_inter_s_card = 0;
                for (const auto& edge: vertex(u).edges)
                    if (s.contains(edge.vertex_id))
                        n_u_inter_s_card++;
                if (n_u_inter_s_card != 1)
                    continue;

                VertexPos n_u_minus_n_s_card = 0;
                VertexPos w = -1;
                for (const auto& edge: vertex(u).edges) {
                    if (!s.contains(edge.vertex_id)
                            && !n_s.contains(edge.vertex_id)) {
                        n_u_minus_n_s_card++;
                        w = edge.vertex_id;
                    }
                }
                if (u_best == -1
                        || n_u_minus_n_s_card_best > n_u_minus_n_s_card) {
                    u_best = u;
                    n_u_minus_n_s_card_best = n_u_minus_n_s_card;
                    w_best = w;
                }
            }
            if (u_best == -1) {
                can_be_removed = false;
                break;
            } else if (n_u_minus_n_s_card_best == 0) {
                can_be_removed = true;
                break;
            } else if (n_u_minus_n_s_card_best == 1) {
                // Update S.
                s.add(w_best);
                if (ws_min > vertex(w_best).weight)
                    ws_min = vertex(w_best).weight;
                // Update N(S).
                if (n_s.contains(w_best))
                    n_s.remove(w_best);
                for (const auto& edge: vertex(w_best).edges) {
                    if (!s.contains(edge.vertex_id)) {
                        n_s.add(edge.vertex_id);
                        if (wns_max < vertex(edge.vertex_id).weight)
                            wns_max = vertex(edge.vertex_id).weight;
                    }
                }
                if (ws_min < wns_max) {
                    can_be_removed = false;
                    break;
                }
                continue;
            } else {
                can_be_removed = false;
                break;
            }
        }

        if (can_be_removed)
            removed_vertices.add(vertex_id);
    }
    //std::cout << folded_vertices.number_of_elements() << std::endl;

    if (removed_vertices.size() == 0)
        return false;

    UnreductionInfo new_unreduction_info;
    new_unreduction_info.original_instance = unreduction_info_.original_instance;

    // Update mandatory_vertices.
    new_unreduction_info.mandatory_vertices = unreduction_info_.mandatory_vertices;
    for (VertexId vertex_id: removed_vertices) {
        for (VertexId vertex_id_2: unreduction_info_.unreduction_operations[vertex_id].out) {
            new_unreduction_info.mandatory_vertices.push_back(vertex_id_2);
        }
    }
    // Update instance and unreduction_operations.
    VertexId new_number_of_vertices = number_of_vertices() - removed_vertices.size();
    InstanceBuilder new_instance_builder;
    new_instance_builder.add_vertices(new_number_of_vertices);
    new_unreduction_info.unreduction_operations = std::vector<UnreductionOperations>(new_number_of_vertices);
    // Add vertices.
    std::vector<VertexId> original2reduced(number_of_vertices(), -1);
    VertexId new_vertex_id = 0;
    for (auto it = removed_vertices.out_begin(); it != removed_vertices.out_end(); ++it) {
        VertexId vertex_id = *it;
        original2reduced[vertex_id] = new_vertex_id;
        new_instance_builder.set_weight(
                new_vertex_id,
                vertex(vertex_id).weight);
        new_unreduction_info.unreduction_operations[new_vertex_id]
            = unreduction_info_.unreduction_operations[vertex_id];
        new_vertex_id++;
    }
    // Add edges.
    for (EdgeId edge_id = 0; edge_id < number_of_edges(); ++edge_id) {
        VertexId vertex_id_1 = edge(edge_id).vertex_id_1;
        VertexId vertex_id_2 = edge(edge_id).vertex_id_2;
        if (removed_vertices.contains(vertex_id_1)
                || removed_vertices.contains(vertex_id_2))
            continue;
        VertexId new_vertex_id_1 = original2reduced[vertex_id_1];
        VertexId new_vertex_id_2 = original2reduced[vertex_id_2];
        new_instance_builder.add_edge(new_vertex_id_1, new_vertex_id_2, 0);
    }

    *this = new_instance_builder.build();
    unreduction_info_ = new_unreduction_info;
    return true;
}

Instance Instance::reduce(ReductionParameters parameters) const
{
    // Initialize reduced instance.
    Instance new_instance_builder = *this;
    new_instance_builder.unreduction_info_ = UnreductionInfo();
    new_instance_builder.unreduction_info_.original_instance = this;
    new_instance_builder.unreduction_info_.unreduction_operations = std::vector<UnreductionOperations>(number_of_vertices());
    for (VertexId vertex_id = 0;
            vertex_id < number_of_vertices();
            ++vertex_id) {
        new_instance_builder.unreduction_info_.unreduction_operations[vertex_id].in.push_back(vertex_id);
    }

    for (Counter round_number = 0;
            round_number < parameters.maximum_number_of_rounds;
            ++round_number) {
        bool found = false;
        found |= new_instance_builder.reduce_pendant_vertices();
        found |= new_instance_builder.reduce_vertex_folding();
        found |= new_instance_builder.reduce_isolated_vertex_removal();
        found |= new_instance_builder.reduce_twin();
        found |= new_instance_builder.reduce_domination();
        found |= new_instance_builder.reduce_unconfined();
        if (!found)
            break;
    }

    new_instance_builder.unreduction_info_.extra_weight = 0;
    for (VertexId orig_vertex_id: new_instance_builder.unreduction_info_.mandatory_vertices)
        new_instance_builder.unreduction_info_.extra_weight += vertex(orig_vertex_id).weight;
    for (VertexId vertex_id = 0;
            vertex_id < new_instance_builder.number_of_vertices();
            ++vertex_id) {
        new_instance_builder.unreduction_info_.extra_weight
            += new_instance_builder.unreduction_info_.unreduction_operations[vertex_id].out.size();
    }

    return new_instance_builder;
}

void stablesolver::stable::init_display(
        const Instance& instance,
        optimizationtools::Info& info)
{
    info.os()
        << "====================================" << std::endl
        << "            StableSolver            " << std::endl
        << "====================================" << std::endl
        << std::endl
        << "Problem" << std::endl
        << "-------" << std::endl
        << "Maximum-weight independent set problem" << std::endl
        << std::endl
        << "Instance" << std::endl
        << "--------" << std::endl;
    instance.print(info.os(), info.verbosity_level());
    info.os() << std::endl;
}

