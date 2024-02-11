#include "stablesolver/stable/reduction.hpp"

#include "stablesolver/stable/instance_builder.hpp"

#include "optimizationtools/containers/indexed_set.hpp"
#include "optimizationtools/containers/indexed_map.hpp"
#include "optimizationtools/containers/doubly_indexed_map.hpp"

using namespace stablesolver::stable;

bool Reduction::reduce_pendant_vertices()
{
    optimizationtools::DoublyIndexedMap fixed_vertices(instance().number_of_vertices(), 2);
    for (VertexId vertex_id = 0;
            vertex_id < instance().number_of_vertices();
            ++vertex_id) {
        if (instance().degree(vertex_id) != 1)
            continue;
        Weight weight = instance().vertex(vertex_id).weight;
        VertexId vertex_id_1 = instance().vertex(vertex_id).edges[0].vertex_id;
        if (instance().vertex(vertex_id_1).weight > weight)
            continue;
        fixed_vertices.set(vertex_id, 1);
        fixed_vertices.set(instance().vertex(vertex_id).edges[0].vertex_id, 0);
    }
    //std::cout << fixed_vertices.number_of_elements() << std::endl;

    if (fixed_vertices.number_of_elements() == 0)
        return false;

    std::vector<UnreductionOperations> new_unreduction_operations;
    std::vector<VertexId> new_mandatory_vertices;

    // Update mandatory_vertices.
    new_mandatory_vertices = mandatory_vertices_;
    for (auto it = fixed_vertices.begin(1); it != fixed_vertices.end(1); ++it) {
        VertexId vertex_id = *it;
        for (VertexId orig_vertex_id: unreduction_operations_[vertex_id].in) {
            new_mandatory_vertices.push_back(orig_vertex_id);
        }
    }
    for (auto it = fixed_vertices.begin(0); it != fixed_vertices.end(0); ++it) {
        VertexId vertex_id = *it;
        for (VertexId orig_vertex_id: unreduction_operations_[vertex_id].out) {
            new_mandatory_vertices.push_back(orig_vertex_id);
        }
    }
    // Create new instance and compute unreduction_operations.
    VertexId new_number_of_vertices = instance().number_of_vertices()
        - fixed_vertices.number_of_elements();
    InstanceBuilder new_instance_builder;
    new_instance_builder.add_vertices(new_number_of_vertices);
    new_unreduction_operations = std::vector<UnreductionOperations>(new_number_of_vertices);
    // Add vertices.
    std::vector<VertexId> original2reduced(instance().number_of_vertices(), -1);
    VertexId new_vertex_id = 0;
    for (auto it = fixed_vertices.out_begin(); it != fixed_vertices.out_end(); ++it) {
        VertexId vertex_id = *it;
        original2reduced[vertex_id] = new_vertex_id;
        new_instance_builder.set_weight(new_vertex_id, instance().vertex(vertex_id).weight);
        new_unreduction_operations[new_vertex_id]
            = unreduction_operations_[vertex_id];
        new_vertex_id++;
    }
    // Add edges.
    for (EdgeId edge_id = 0; edge_id < instance().number_of_edges(); ++edge_id) {
        VertexId vertex_id_1 = instance().edge(edge_id).vertex_id_1;
        VertexId vertex_id_2 = instance().edge(edge_id).vertex_id_2;
        VertexId new_vertex_id_1 = original2reduced[vertex_id_1];
        VertexId new_vertex_id_2 = original2reduced[vertex_id_2];
        if (new_vertex_id_1 != -1 && new_vertex_id_2 != -1) {
            new_instance_builder.add_edge(
                    new_vertex_id_1,
                    new_vertex_id_2,
                    0);
        }
    }

    unreduction_operations_ = new_unreduction_operations;
    mandatory_vertices_ = new_mandatory_vertices;
    instance_ = new_instance_builder.build();
    return true;
}

bool Reduction::reduce_isolated_vertex_removal()
{
    //std::cout << "Isolated vertex removal..." << std::endl;
    optimizationtools::IndexedSet neighbors(instance().number_of_vertices());
    optimizationtools::DoublyIndexedMap fixed_vertices(instance().number_of_vertices(), 2);
    for (VertexId vertex_id = 0;
            vertex_id < instance().number_of_vertices();
            ++vertex_id) {
        if (fixed_vertices.contains(vertex_id))
            continue;
        bool neighbors_clique = true;
        for (const auto& edge: instance().vertex(vertex_id).edges) {
            if (fixed_vertices.contains(edge.vertex_id))
                continue;
            // Check if all neighbors of v are neighbors of edge.v.
            neighbors.clear();
            neighbors.add(edge.vertex_id);
            for (const auto& edge_2: instance().vertex(edge.vertex_id).edges)
                if (!fixed_vertices.contains(edge_2.vertex_id))
                    neighbors.add(edge_2.vertex_id);
            for (const auto& edge: instance().vertex(vertex_id).edges) {
                if (!fixed_vertices.contains(edge.vertex_id)) {
                    if (!neighbors.contains(edge.vertex_id)
                            || instance().vertex(vertex_id).weight
                            < instance().vertex(edge.vertex_id).weight) {
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
            for (const auto& edge: instance().vertex(vertex_id).edges) {
                fixed_vertices.set(edge.vertex_id, 0);
            }
        }
    }
    //std::cout << fixed_vertices.number_of_elements() << std::endl;

    if (fixed_vertices.number_of_elements() == 0)
        return false;

    std::vector<UnreductionOperations> new_unreduction_operations;
    std::vector<VertexId> new_mandatory_vertices;

    // Update mandatory_vertices.
    new_mandatory_vertices = mandatory_vertices_;
    for (auto it = fixed_vertices.begin(1); it != fixed_vertices.end(1); ++it) {
        VertexId vertex_id = *it;
        for (VertexId orig_vertex_id: unreduction_operations_[vertex_id].in) {
            new_mandatory_vertices.push_back(orig_vertex_id);
        }
    }
    for (auto it = fixed_vertices.begin(0); it != fixed_vertices.end(0); ++it) {
        VertexId vertex_id = *it;
        for (VertexId orig_vertex_id: unreduction_operations_[vertex_id].out) {
            new_mandatory_vertices.push_back(orig_vertex_id);
        }
    }
    // Create new instance and compute unreduction_operations.
    VertexId new_number_of_vertices = instance().number_of_vertices()
        - fixed_vertices.number_of_elements();
    InstanceBuilder new_instance_builder;
    new_instance_builder.add_vertices(new_number_of_vertices);
    new_unreduction_operations = std::vector<UnreductionOperations>(new_number_of_vertices);
    // Add vertices.
    std::vector<VertexId> original2reduced(instance().number_of_vertices(), -1);
    VertexId new_vertex_id = 0;
    for (auto it = fixed_vertices.out_begin(); it != fixed_vertices.out_end(); ++it) {
        VertexId vertex_id = *it;
        original2reduced[vertex_id] = new_vertex_id;
        new_instance_builder.set_weight(new_vertex_id, instance().vertex(vertex_id).weight);
        new_unreduction_operations[new_vertex_id]
            = unreduction_operations_[vertex_id];
        new_vertex_id++;
    }
    // Add edges.
    for (EdgeId edge_id = 0;
            edge_id < instance().number_of_edges();
            ++edge_id) {
        VertexId vertex_id_1 = instance().edge(edge_id).vertex_id_1;
        VertexId vertex_id_2 = instance().edge(edge_id).vertex_id_2;
        VertexId new_vertex_id_1 = original2reduced[vertex_id_1];
        VertexId new_vertex_id_2 = original2reduced[vertex_id_2];
        if (new_vertex_id_1 != -1 && new_vertex_id_2 != -1) {
            new_instance_builder.add_edge(new_vertex_id_1, new_vertex_id_2, 0);
        }
    }

    unreduction_operations_ = new_unreduction_operations;
    mandatory_vertices_ = new_mandatory_vertices;
    instance_ = new_instance_builder.build();
    return true;
}

bool Reduction::reduce_vertex_folding()
{
    //std::cout << "Vertex folding..." << std::endl;
    optimizationtools::IndexedSet folded_vertices(instance().number_of_vertices());
    std::vector<std::tuple<VertexId, VertexId, VertexId>> folded_vertices_list;
    for (VertexId vertex_id = 0;
            vertex_id < instance().number_of_vertices();
            ++vertex_id) {
        if (instance().degree(vertex_id) != 2)
            continue;
        VertexId vertex_id_1 = instance().vertex(vertex_id).edges[0].vertex_id;
        VertexId vertex_id_2 = instance().vertex(vertex_id).edges[1].vertex_id;
        if (folded_vertices.contains(vertex_id)
                || folded_vertices.contains(vertex_id_1)
                || folded_vertices.contains(vertex_id_2))
            continue;
        if (instance().vertex(vertex_id).weight != instance().vertex(vertex_id_1).weight
                || instance().vertex(vertex_id).weight != instance().vertex(vertex_id_2).weight)
            continue;
        // Check if there exists an edge (vertex_id_1, vertex_id_2).
        bool ok = true;
        for (const auto& edge: instance().vertex(vertex_id_1).edges) {
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

    std::vector<UnreductionOperations> new_unreduction_operations;
    std::vector<VertexId> new_mandatory_vertices;

    // Update mandatory_vertices.
    new_mandatory_vertices = mandatory_vertices_;
    // Update instance and unreduction_operations.
    VertexId new_number_of_vertices = instance().number_of_vertices() - folded_vertices.size() + folded_vertices_list.size();
    InstanceBuilder new_instance_builder;
    new_instance_builder.add_vertices(new_number_of_vertices);
    new_unreduction_operations = std::vector<UnreductionOperations>(new_number_of_vertices);
    // Add vertices.
    std::vector<VertexId> original2reduced(instance().number_of_vertices(), -1);
    VertexId new_vertex_id = 0;
    for (auto it = folded_vertices.out_begin(); it != folded_vertices.out_end(); ++it) {
        VertexId vertex_id = *it;
        original2reduced[vertex_id] = new_vertex_id;
        new_instance_builder.set_weight(new_vertex_id, instance().vertex(vertex_id).weight);
        new_unreduction_operations[new_vertex_id]
            = unreduction_operations_[vertex_id];
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
        new_instance_builder.set_weight(new_vertex_id, instance().vertex(vertex_id).weight);

        for (VertexId vertex_id_3: unreduction_operations_[vertex_id].out)
            new_unreduction_operations[new_vertex_id].in.push_back(vertex_id_3);
        for (VertexId vertex_id_3: unreduction_operations_[vertex_id].in)
            new_unreduction_operations[new_vertex_id].out.push_back(vertex_id_3);

        for (VertexId vertex_id_3: unreduction_operations_[vertex_id_1].in)
            new_unreduction_operations[new_vertex_id].in.push_back(vertex_id_3);
        for (VertexId vertex_id_3: unreduction_operations_[vertex_id_1].out)
            new_unreduction_operations[new_vertex_id].out.push_back(vertex_id_3);

        for (VertexId vertex_id_3: unreduction_operations_[vertex_id_2].in)
            new_unreduction_operations[new_vertex_id].in.push_back(vertex_id_3);
        for (VertexId vertex_id_3: unreduction_operations_[vertex_id_2].out)
            new_unreduction_operations[new_vertex_id].out.push_back(vertex_id_3);

        new_vertex_id++;
    }
    // Add edges.
    for (EdgeId e = 0; e < instance().number_of_edges(); ++e) {
        VertexId vertex_id_1 = instance().edge(e).vertex_id_1;
        VertexId vertex_id_2 = instance().edge(e).vertex_id_2;
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
        for (const auto& edge: instance().vertex(vertex_id_1).edges)
            if (edge.vertex_id != vertex_id)
                if (!folded_vertices.contains(edge.vertex_id)
                        || new_vertex_id < original2reduced[edge.vertex_id])
                    neighbors_tmp.add(original2reduced[edge.vertex_id]);
        for (const auto& edge: instance().vertex(vertex_id_2).edges)
            if (edge.vertex_id != vertex_id)
                if (!folded_vertices.contains(edge.vertex_id)
                        || new_vertex_id < original2reduced[edge.vertex_id])
                    neighbors_tmp.add(original2reduced[edge.vertex_id]);
        for (VertexId vertex_id_3_new: neighbors_tmp)
            new_instance_builder.add_edge(new_vertex_id, vertex_id_3_new, 0);
    }

    unreduction_operations_ = new_unreduction_operations;
    mandatory_vertices_ = new_mandatory_vertices;
    instance_ = new_instance_builder.build();
    return true;
}

bool Reduction::reduce_twin()
{
    //std::cout << "Vertex folding..." << std::endl;
    // 0: removed
    // 1: added
    // 2: folded
    optimizationtools::DoublyIndexedMap modified_vertices(instance().number_of_vertices(), 3);
    optimizationtools::IndexedMap<VertexPos> twin_candidates(instance().number_of_vertices(), 0);
    std::vector<std::tuple<VertexId, VertexId, VertexId, VertexId, VertexId>> folded_vertices_list;
    for (VertexId vertex_id = 0;
            vertex_id < instance().number_of_vertices();
            ++vertex_id) {
        if (instance().degree(vertex_id) != 3)
            continue;
        VertexId vertex_id_1 = instance().vertex(vertex_id).edges[0].vertex_id;
        VertexId vertex_id_2 = instance().vertex(vertex_id).edges[1].vertex_id;
        VertexId vertex_id_3 = instance().vertex(vertex_id).edges[2].vertex_id;
        if (modified_vertices.contains(vertex_id)
                || modified_vertices.contains(vertex_id_1)
                || modified_vertices.contains(vertex_id_2)
                || modified_vertices.contains(vertex_id_3))
            continue;
        Weight weight = instance().vertex(vertex_id).weight;
        if (instance().vertex(vertex_id_1).weight != weight
                || instance().vertex(vertex_id_2).weight != weight
                || instance().vertex(vertex_id_3).weight != weight)
            continue;
        twin_candidates.clear();
        for (const auto& edge: instance().vertex(vertex_id).edges) {
            for (const auto& edge_2: instance().vertex(edge.vertex_id).edges) {
                if (instance().degree(edge_2.vertex_id) != 3)
                    continue;
                if (edge_2.vertex_id == vertex_id)
                    continue;
                if (instance().vertex(edge_2.vertex_id).weight != weight)
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
        for (const auto& edge: instance().vertex(vertex_id_1).edges)
            if (edge.vertex_id == vertex_id_2 || edge.vertex_id == vertex_id_3)
                has_edge = true;
        for (const auto& edge: instance().vertex(vertex_id_2).edges)
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

    std::vector<UnreductionOperations> new_unreduction_operations;
    std::vector<VertexId> new_mandatory_vertices;

    // Update mandatory_vertices.
    new_mandatory_vertices = mandatory_vertices_;
    for (auto it = modified_vertices.begin(1); it != modified_vertices.end(1); ++it) {
        VertexId vertex_id = *it;
        for (VertexId orig_vertex_id: unreduction_operations_[vertex_id].in) {
            new_mandatory_vertices.push_back(orig_vertex_id);
        }
    }
    for (auto it = modified_vertices.begin(0); it != modified_vertices.end(0); ++it) {
        VertexId vertex_id = *it;
        for (VertexId orig_vertex_id: unreduction_operations_[vertex_id].out) {
            new_mandatory_vertices.push_back(orig_vertex_id);
        }
    }
    // Create new instance and compute unreduction_operations.
    VertexId new_number_of_vertices = instance().number_of_vertices()
        - modified_vertices.number_of_elements(0)
        - modified_vertices.number_of_elements(1)
        - modified_vertices.number_of_elements(2)
        + modified_vertices.number_of_elements(2) / 5;
    InstanceBuilder new_instance_builder;
    new_instance_builder.add_vertices(new_number_of_vertices);
    new_unreduction_operations = std::vector<UnreductionOperations>(new_number_of_vertices);
    // Add vertices.
    std::vector<VertexId> original2reduced(instance().number_of_vertices(), -1);
    VertexId new_vertex_id = 0;
    for (auto it = modified_vertices.out_begin();
            it != modified_vertices.out_end();
            ++it) {
        VertexId vertex_id = *it;
        original2reduced[vertex_id] = new_vertex_id;
        new_instance_builder.set_weight(new_vertex_id, instance().vertex(vertex_id).weight);
        new_unreduction_operations[new_vertex_id]
            = unreduction_operations_[vertex_id];
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
        new_instance_builder.set_weight(new_vertex_id, instance().vertex(vertex_id).weight);

        for (VertexId vertex_id_tmp: unreduction_operations_[vertex_id].out)
            new_unreduction_operations[new_vertex_id].in.push_back(vertex_id_tmp);
        for (VertexId vertex_id_tmp: unreduction_operations_[vertex_id].in)
            new_unreduction_operations[new_vertex_id].out.push_back(vertex_id_tmp);

        for (VertexId vertex_id_tmp: unreduction_operations_[vertex_id_twin].out)
            new_unreduction_operations[new_vertex_id].in.push_back(vertex_id_tmp);
        for (VertexId vertex_id_tmp: unreduction_operations_[vertex_id_twin].in)
            new_unreduction_operations[new_vertex_id].out.push_back(vertex_id_tmp);

        for (VertexId vertex_id_tmp: unreduction_operations_[vertex_id_1].in)
            new_unreduction_operations[new_vertex_id].in.push_back(vertex_id_tmp);
        for (VertexId vertex_id_tmp: unreduction_operations_[vertex_id_1].out)
            new_unreduction_operations[new_vertex_id].out.push_back(vertex_id_tmp);

        for (VertexId vertex_id_tmp: unreduction_operations_[vertex_id_2].in)
            new_unreduction_operations[new_vertex_id].in.push_back(vertex_id_tmp);
        for (VertexId vertex_id_tmp: unreduction_operations_[vertex_id_2].out)
            new_unreduction_operations[new_vertex_id].out.push_back(vertex_id_tmp);

        for (VertexId vertex_id_tmp: unreduction_operations_[vertex_id_3].in)
            new_unreduction_operations[new_vertex_id].in.push_back(vertex_id_tmp);
        for (VertexId vertex_id_tmp: unreduction_operations_[vertex_id_3].out)
            new_unreduction_operations[new_vertex_id].out.push_back(vertex_id_tmp);

        new_vertex_id++;
    }
    // Add edges.
    for (EdgeId edge_id = 0;
            edge_id < instance().number_of_edges();
            ++edge_id) {
        VertexId vertex_id_1 = instance().edge(edge_id).vertex_id_1;
        VertexId vertex_id_2 = instance().edge(edge_id).vertex_id_2;
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
        for (const auto& edge: instance().vertex(vertex_id_1).edges)
            if (edge.vertex_id != vertex_id
                    && edge.vertex_id != vertex_id_twin)
                if (!modified_vertices.contains(edge.vertex_id)
                        || new_vertex_id < original2reduced[edge.vertex_id])
                    neighbors_tmp.add(original2reduced[edge.vertex_id]);
        for (const auto& edge: instance().vertex(vertex_id_2).edges)
            if (edge.vertex_id != vertex_id
                    && edge.vertex_id != vertex_id_twin)
                if (!modified_vertices.contains(edge.vertex_id)
                        || new_vertex_id < original2reduced[edge.vertex_id])
                    neighbors_tmp.add(original2reduced[edge.vertex_id]);
        for (const auto& edge: instance().vertex(vertex_id_3).edges)
            if (edge.vertex_id != vertex_id
                    && edge.vertex_id != vertex_id_twin)
                if (!modified_vertices.contains(edge.vertex_id)
                        || new_vertex_id < original2reduced[edge.vertex_id])
                    neighbors_tmp.add(original2reduced[edge.vertex_id]);
        for (VertexId vertex_id_tmp: neighbors_tmp)
            new_instance_builder.add_edge(new_vertex_id, vertex_id_tmp, 0);
    }

    unreduction_operations_ = new_unreduction_operations;
    mandatory_vertices_ = new_mandatory_vertices;
    instance_ = new_instance_builder.build();
    return true;
}

bool Reduction::reduce_domination()
{
    //std::cout << "Vertex folding..." << std::endl;
    optimizationtools::IndexedSet removed_vertices(instance().number_of_vertices());
    optimizationtools::IndexedSet neighbors(instance().number_of_vertices());
    for (VertexId vertex_id = 0;
            vertex_id < instance().number_of_vertices();
            ++vertex_id) {
        Weight weight = instance().vertex(vertex_id).weight;
        neighbors.clear();
        for (const auto& edge: instance().vertex(vertex_id).edges)
            neighbors.add(edge.vertex_id);
        bool can_be_removed = false;
        for (const auto& edge: instance().vertex(vertex_id).edges) {
            if (instance().vertex(edge.vertex_id).weight < weight)
                continue;
            if (removed_vertices.contains(edge.vertex_id))
                continue;
            bool dominates = true;
            for (const auto& edge_2: instance().vertex(edge.vertex_id).edges) {
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

    std::vector<UnreductionOperations> new_unreduction_operations;
    std::vector<VertexId> new_mandatory_vertices;

    // Update mandatory_vertices.
    new_mandatory_vertices = mandatory_vertices_;
    for (VertexId vertex_id: removed_vertices) {
        for (VertexId orig_vertex_id: unreduction_operations_[vertex_id].out) {
            new_mandatory_vertices.push_back(orig_vertex_id);
        }
    }
    // Update instance and unreduction_operations.
    VertexId new_number_of_vertices = instance().number_of_vertices() - removed_vertices.size();
    InstanceBuilder new_instance_builder;
    new_instance_builder.add_vertices(new_number_of_vertices);
    new_unreduction_operations = std::vector<UnreductionOperations>(new_number_of_vertices);
    // Add vertices.
    std::vector<VertexId> original2reduced(instance().number_of_vertices(), -1);
    VertexId new_vertex_id = 0;
    for (auto it = removed_vertices.out_begin(); it != removed_vertices.out_end(); ++it) {
        VertexId vertex_id = *it;
        original2reduced[vertex_id] = new_vertex_id;
        new_instance_builder.set_weight(new_vertex_id, instance().vertex(vertex_id).weight);
        new_unreduction_operations[new_vertex_id]
            = unreduction_operations_[vertex_id];
        new_vertex_id++;
    }
    // Add edges.
    for (EdgeId edge_id = 0;
            edge_id < instance().number_of_edges();
            ++edge_id) {
        VertexId vertex_id_1 = instance().edge(edge_id).vertex_id_1;
        VertexId vertex_id_2 = instance().edge(edge_id).vertex_id_2;
        if (removed_vertices.contains(vertex_id_1) || removed_vertices.contains(vertex_id_2))
            continue;
        VertexId new_vertex_id_1 = original2reduced[vertex_id_1];
        VertexId new_vertex_id_2 = original2reduced[vertex_id_2];
        new_instance_builder.add_edge(new_vertex_id_1, new_vertex_id_2, 0);
    }

    unreduction_operations_ = new_unreduction_operations;
    mandatory_vertices_ = new_mandatory_vertices;
    instance_ = new_instance_builder.build();
    return true;
}

bool Reduction::reduce_unconfined()
{
    //std::cout << "Vertex folding..." << std::endl;
    optimizationtools::IndexedSet removed_vertices(instance().number_of_vertices());
    optimizationtools::IndexedSet s(instance().number_of_vertices());
    optimizationtools::IndexedSet n_s(instance().number_of_vertices());
    for (VertexId vertex_id = 0;
            vertex_id < instance().number_of_vertices();
            ++vertex_id) {
        // Minimum weight in S.
        // The unconfined reduction rule remains true while the minimum weight
        // in S is greater or equal to the maximum weight in N(S).
        Weight ws_min = instance().vertex(vertex_id).weight;
        Weight wns_max = 0;

        s.clear();
        n_s.clear();

        // Update S.
        s.add(vertex_id);
        // Update N(S).
        for (const auto& edge: instance().vertex(vertex_id).edges) {
            n_s.add(edge.vertex_id);
            if (wns_max < instance().vertex(edge.vertex_id).weight)
                wns_max = instance().vertex(edge.vertex_id).weight;
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
                for (const auto& edge: instance().vertex(u).edges)
                    if (s.contains(edge.vertex_id))
                        n_u_inter_s_card++;
                if (n_u_inter_s_card != 1)
                    continue;

                VertexPos n_u_minus_n_s_card = 0;
                VertexPos w = -1;
                for (const auto& edge: instance().vertex(u).edges) {
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
                if (ws_min > instance().vertex(w_best).weight)
                    ws_min = instance().vertex(w_best).weight;
                // Update N(S).
                if (n_s.contains(w_best))
                    n_s.remove(w_best);
                for (const auto& edge: instance().vertex(w_best).edges) {
                    if (!s.contains(edge.vertex_id)) {
                        n_s.add(edge.vertex_id);
                        if (wns_max < instance().vertex(edge.vertex_id).weight)
                            wns_max = instance().vertex(edge.vertex_id).weight;
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

    std::vector<UnreductionOperations> new_unreduction_operations;
    std::vector<VertexId> new_mandatory_vertices;

    // Update mandatory_vertices.
    new_mandatory_vertices = mandatory_vertices_;
    for (VertexId vertex_id: removed_vertices) {
        for (VertexId vertex_id_2: unreduction_operations_[vertex_id].out) {
            new_mandatory_vertices.push_back(vertex_id_2);
        }
    }
    // Update instance and unreduction_operations.
    VertexId new_number_of_vertices = instance().number_of_vertices() - removed_vertices.size();
    InstanceBuilder new_instance_builder;
    new_instance_builder.add_vertices(new_number_of_vertices);
    new_unreduction_operations = std::vector<UnreductionOperations>(new_number_of_vertices);
    // Add vertices.
    std::vector<VertexId> original2reduced(instance().number_of_vertices(), -1);
    VertexId new_vertex_id = 0;
    for (auto it = removed_vertices.out_begin(); it != removed_vertices.out_end(); ++it) {
        VertexId vertex_id = *it;
        original2reduced[vertex_id] = new_vertex_id;
        new_instance_builder.set_weight(
                new_vertex_id,
                instance().vertex(vertex_id).weight);
        new_unreduction_operations[new_vertex_id]
            = unreduction_operations_[vertex_id];
        new_vertex_id++;
    }
    // Add edges.
    for (EdgeId edge_id = 0; edge_id < instance().number_of_edges(); ++edge_id) {
        VertexId vertex_id_1 = instance().edge(edge_id).vertex_id_1;
        VertexId vertex_id_2 = instance().edge(edge_id).vertex_id_2;
        if (removed_vertices.contains(vertex_id_1)
                || removed_vertices.contains(vertex_id_2))
            continue;
        VertexId new_vertex_id_1 = original2reduced[vertex_id_1];
        VertexId new_vertex_id_2 = original2reduced[vertex_id_2];
        new_instance_builder.add_edge(new_vertex_id_1, new_vertex_id_2, 0);
    }

    unreduction_operations_ = new_unreduction_operations;
    mandatory_vertices_ = new_mandatory_vertices;
    instance_ = new_instance_builder.build();
    return true;
}

Reduction::Reduction(
        const Instance& instance,
        const ReductionParameters& parameters):
    original_instance_(&instance),
    instance_(instance)
{
    // Initialize reduced instance.
    unreduction_operations_ = std::vector<UnreductionOperations>(instance.number_of_vertices());
    for (VertexId vertex_id = 0;
            vertex_id < instance.number_of_vertices();
            ++vertex_id) {
        unreduction_operations_[vertex_id].in.push_back(vertex_id);
    }

    for (Counter round_number = 0;
            round_number < parameters.maximum_number_of_rounds;
            ++round_number) {
        bool found = false;
        found |= reduce_pendant_vertices();
        found |= reduce_vertex_folding();
        found |= reduce_isolated_vertex_removal();
        found |= reduce_twin();
        found |= reduce_domination();
        found |= reduce_unconfined();
        if (!found)
            break;
    }

    extra_weight_ = 0;
    for (VertexId orig_vertex_id: mandatory_vertices_)
        extra_weight_ += instance.vertex(orig_vertex_id).weight;
}

Solution Reduction::unreduce_solution(
        const Solution& solution) const
{
    Solution new_solution(*original_instance_);

    for (VertexId vertex_id: mandatory_vertices_)
        new_solution.add(vertex_id);

    for (VertexId vertex_id = 0;
            vertex_id < instance().number_of_vertices();
            ++vertex_id) {
        if (solution.contains(vertex_id))
            for (VertexId vertex_id_2: unreduction_operations_[vertex_id].in)
                new_solution.add(vertex_id_2);
        if (!solution.contains(vertex_id))
            for (VertexId vertex_id_2: unreduction_operations_[vertex_id].out)
                new_solution.add(vertex_id_2);
    }

    return new_solution;
}

Weight Reduction::unreduce_bound(
        Weight bound) const
{
    return extra_weight_ + bound;
}
