#include "stablesolver/instance.hpp"

#include "optimizationtools/containers/indexed_map.hpp"
#include "optimizationtools/containers/doubly_indexed_map.hpp"

#include <sstream>
#include <iomanip>
#include <thread>

using namespace stablesolver;

Instance::Instance(std::string instance_path, std::string format)
{
    std::ifstream file(instance_path);
    if (!file.good()) {
        throw std::runtime_error(
                "Unable to open file \"" + instance_path + "\".");
    }

    if (format == "dimacs1992") {
        read_dimacs1992(file);
    } else if (format == "dimacs2010") {
        read_dimacs2010(file);
    } else if (format == "matrixmarket") {
        read_matrixmarket(file);
    } else if (format == "chaco") {
        read_chaco(file);
    } else if (format == "snap") {
        read_snap(file);
    } else {
        throw std::invalid_argument(
                "Unknown instance format \"" + format + "\".");
    }

    compute_components();
}

Instance::Instance(VertexId number_of_vertices)
{
    for (VertexId vertex_id = 0; vertex_id < number_of_vertices; ++vertex_id)
        add_vertex();
}

void Instance::add_vertex(Weight weight)
{
    Vertex vertex;
    vertex.weight = weight;
    total_weight_ += weight;
    vertices_.push_back(vertex);
}

void Instance::add_edge(
        VertexId vertex_id_1,
        VertexId vertex_id_2,
        int check_duplicate)
{
    if (vertex_id_1 == vertex_id_2) {
        std::cerr << "\033[33m" << "WARNING, loop (" << vertex_id_1 << ", " << vertex_id_2 << ") ignored." << "\033[0m" << std::endl;
        return;
    }

    if (check_duplicate > 0) {
        for (const auto& edge: vertex(vertex_id_1).edges) {
            if (edge.vertex_id == vertex_id_2) {
                if (check_duplicate == 1) {
                    return;
                } else {
                    throw std::runtime_error(
                            "Duplicate edge: ("
                            + std::to_string(vertex_id_1)
                            + ","
                            + std::to_string(vertex_id_2)
                            + ").");
                }
            }
        }
    }

    EdgeId edge_id = edges_.size();

    Edge e;
    e.vertex_id_1 = vertex_id_1;
    e.vertex_id_2 = vertex_id_2;
    edges_.push_back(e);

    VertexEdge ve1;
    ve1.edge_id = edge_id;
    ve1.vertex_id = vertex_id_2;
    vertices_[vertex_id_1].edges.push_back(ve1);
    if (maximum_degree_ < (VertexId)vertices_[vertex_id_1].edges.size())
        maximum_degree_ = vertices_[vertex_id_1].edges.size();

    VertexEdge ve2;
    ve2.edge_id = edge_id;
    ve2.vertex_id = vertex_id_1;
    vertices_[vertex_id_2].edges.push_back(ve2);
    if (maximum_degree_ < (VertexId)vertices_[vertex_id_2].edges.size())
        maximum_degree_ = vertices_[vertex_id_2].edges.size();
}

void Instance::set_weight(
        VertexId vertex_id,
        Weight weight)
{
    if (weight < 0) {
        throw std::invalid_argument("Set negative weight '"
                + std::to_string(weight)
                + "' to vertex '"
                + std::to_string(vertex_id)
                + "'.");
    }
    total_weight_ -= vertex(vertex_id).weight;
    vertices_[vertex_id].weight = weight;
    total_weight_ += vertex(vertex_id).weight;
}

void Instance::set_unweighted()
{
    for (VertexId vertex_id = 0; vertex_id < number_of_vertices(); ++vertex_id)
        vertices_[vertex_id].weight = 1;
    total_weight_ = number_of_vertices();
}

void Instance::read_dimacs1992(std::ifstream& file)
{
    std::string tmp;
    std::vector<std::string> line;

    while (getline(file, tmp)) {
        line = optimizationtools::split(tmp, ' ');
        if (line.size() == 0) {
        } else if (line[0] == "c") {
            if (name_ == "")
                name_ = line.back();
        } else if (line[0] == "p") {
            VertexId number_of_vertices = stol(line[2]);
            vertices_.resize(number_of_vertices);
            total_weight_ = number_of_vertices;
        } else if (line[0] == "n") {
            VertexId vertex_id = stol(line[1]) - 1;
            Weight weight = stol(line[2]);
            set_weight(vertex_id, weight);
        } else if (line[0] == "e") {
            VertexId vertex_id_1 = stol(line[1]) - 1;
            VertexId vertex_id_2 = stol(line[2]) - 1;
            add_edge(vertex_id_1, vertex_id_2);
        }
    }
}

void Instance::read_dimacs2010(std::ifstream& file)
{
    std::string tmp;
    std::vector<std::string> line;
    bool first = true;
    VertexId vertex_id = -1;
    while (vertex_id != number_of_vertices()) {
        getline(file, tmp);
        //std::cout << tmp << std::endl;
        line = optimizationtools::split(tmp, ' ');
        if (tmp[0] == '%')
            continue;
        if (first) {
            VertexId number_of_vertices = stol(line[0]);
            vertices_.resize(number_of_vertices);
            total_weight_ = number_of_vertices;
            first = false;
            vertex_id = 0;
        } else {
            for (std::string str: line) {
                VertexId vertex_id_2 = stol(str) - 1;
                if (vertex_id_2 > vertex_id)
                    add_edge(vertex_id, vertex_id_2);
            }
            vertex_id++;
        }
    }
}

void Instance::read_matrixmarket(std::ifstream& file)
{
    std::string tmp;
    std::vector<std::string> line;
    do {
        getline(file, tmp);
    } while (tmp[0] == '%');
    line = optimizationtools::split(tmp, ' ');
    VertexId n = stol(line[0]);
    vertices_.resize(n);
    total_weight_ = n;

    while (getline(file, tmp)) {
        line = optimizationtools::split(tmp, ' ');
        VertexId vertex_id_1 = stol(line[0]) - 1;
        VertexId vertex_id_2 = stol(line[1]) - 1;
        add_edge(vertex_id_1, vertex_id_2);
    }
}

void Instance::read_chaco(std::ifstream& file)
{
    std::string tmp;
    std::vector<std::string> line;

    getline(file, tmp);
    line = optimizationtools::split(tmp, ' ');
    VertexId number_of_vertices = stol(line[0]);
    vertices_.resize(number_of_vertices);

    for (VertexId v = 0; v < number_of_vertices; ++v) {
        getline(file, tmp);
        line = optimizationtools::split(tmp, ' ');
        for (std::string str: line) {
            VertexId vertex_id_2 = stol(str) - 1;
            if (vertex_id_2 > v)
                add_edge(v, vertex_id_2);
        }
    }
}

void Instance::read_snap(std::ifstream& file)
{
    std::string tmp;
    std::vector<std::string> line;
    do {
        getline(file, tmp);
    } while (tmp[0] == '#');

    VertexId vertex_id_1 = -1;
    VertexId vertex_id_2 = -1;
    for (;;) {
        file >> vertex_id_1 >> vertex_id_2;
        if (file.eof())
            break;
        while (std::max(vertex_id_1, vertex_id_2) >= number_of_vertices())
            add_vertex();
        add_edge(vertex_id_1, vertex_id_2);
    }
}

Instance Instance::complementary()
{
    Instance instance(number_of_vertices());
    optimizationtools::IndexedSet neighbors(number_of_vertices());

    for (VertexId vertex_id = 0;
            vertex_id < number_of_vertices();
            ++vertex_id) {
        instance.set_weight(vertex_id, vertex(vertex_id).weight);
        neighbors.clear();
        neighbors.add(vertex_id);
        for (const auto& edge: vertex(vertex_id).edges)
            neighbors.add(edge.vertex_id);
        for (auto it = neighbors.out_begin(); it != neighbors.out_end(); ++it)
            if (*it > vertex_id)
                instance.add_edge(vertex_id, *it);
    }

    instance.compute_components();
    return instance;
}

void Instance::compute_components()
{
    //std::cout << "compute_components" << std::endl;
    std::vector<VertexId> stack;
    VertexId vertex_id_0 = 0;
    for (ComponentId c = 0;; ++c) {
        while (vertex_id_0 < number_of_vertices()
                && (vertex(vertex_id_0).component != -1))
            vertex_id_0++;
        if (vertex_id_0 == number_of_vertices())
            break;
        //std::cout << "c " << c << " v " << v << std::endl;
        Component component;
        component.id = c;
        stack.clear();
        stack.push_back(vertex_id_0);
        vertices_[vertex_id_0].component = c;
        while (!stack.empty()) {
            VertexId vertex_id = stack.back();
            stack.pop_back();
            for (const auto& edge: vertex(vertex_id).edges) {
                edges_[edge.edge_id].component = c;
                if (vertex(edge.vertex_id).component != -1)
                    continue;
                vertices_[edge.vertex_id].component = c;
                stack.push_back(edge.vertex_id);
            }
        }
        components_.push_back(component);
    }

    for (VertexId vertex_id = 0;
            vertex_id < number_of_vertices();
            ++vertex_id) {
        if (vertex(vertex_id).component != -1)
            components_[vertex(vertex_id).component].vertices.push_back(vertex_id);
    }
    for (EdgeId edge_id = 0; edge_id < number_of_edges(); ++edge_id)
        components_[edge(edge_id).component].edges.push_back(edge_id);

    //for (const auto& component: components_) {
    //    std::cout << "Component " << component.id
    //        << " n " << component.vertices.size()
    //        << " m " << component.edges.size()
    //        << std::endl;
    //    //std::cout << "Vertices:";
    //    //for (VertexId v: component.vertices)
    //    //    std::cout << " " << v;
    //    //std::cout << std::endl;
    //    //std::cout << "Edges:";
    //    //for (EdgeId e: component.edges)
    //    //    std::cout << " " << edge(e).vertex_id_1 << "," << edge(e).vertex_id_2;
    //    //std::cout << std::endl;
    //}
}

ReductionOutput Instance::reduce_pendant_vertices(
        const ReductionOutput& reduction_output_old)
{
    const Instance& instance = *reduction_output_old.instance;
    optimizationtools::IndexedSet neighbors(instance.number_of_vertices());
    optimizationtools::DoublyIndexedMap fixed_vertices(instance.number_of_vertices(), 2);
    for (VertexId vertex_id = 0;
            vertex_id < instance.number_of_vertices();
            ++vertex_id) {
        if (instance.degree(vertex_id) != 1)
            continue;
        Weight weight = instance.vertex(vertex_id).weight;
        VertexId vertex_id_1 = instance.vertex(vertex_id).edges[0].vertex_id;
        if (instance.vertex(vertex_id_1).weight > weight)
            continue;
        fixed_vertices.set(vertex_id, 1);
        fixed_vertices.set(instance.vertex(vertex_id).edges[0].vertex_id, 0);
    }
    //std::cout << fixed_vertices.number_of_elements() << std::endl;

    ReductionOutput reduction_output;
    if (fixed_vertices.number_of_elements() == 0)
        return reduction_output;

    // Update mandatory_vertices.
    reduction_output.mandatory_vertices = reduction_output_old.mandatory_vertices;
    for (auto it = fixed_vertices.begin(1); it != fixed_vertices.end(1); ++it) {
        VertexId vertex_id = *it;
        for (VertexId vertex_id_2: reduction_output_old.unreduction_operations[vertex_id].in) {
            reduction_output.mandatory_vertices.push_back(vertex_id_2);
        }
    }
    for (auto it = fixed_vertices.begin(0); it != fixed_vertices.end(0); ++it) {
        VertexId vertex_id = *it;
        for (VertexId vertex_id_2: reduction_output_old.unreduction_operations[vertex_id].out) {
            reduction_output.mandatory_vertices.push_back(vertex_id_2);
        }
    }
    // Update instance and unreduction_operations.
    VertexId n = instance.number_of_vertices() - fixed_vertices.number_of_elements();
    reduction_output.instance = new Instance(n);
    reduction_output.unreduction_operations = std::vector<UnreductionOperations>(n);
    // Add vertices.
    std::vector<VertexId> original2reduced(instance.number_of_vertices(), -1);
    VertexId vertex_id_new = 0;
    for (auto it = fixed_vertices.out_begin(); it != fixed_vertices.out_end(); ++it) {
        VertexId vertex_id = *it;
        original2reduced[vertex_id] = vertex_id_new;
        reduction_output.instance->set_weight(vertex_id_new, instance.vertex(vertex_id).weight);
        reduction_output.unreduction_operations[vertex_id_new]
            = reduction_output_old.unreduction_operations[vertex_id];
        vertex_id_new++;
    }
    // Add edges.
    for (EdgeId edge_id = 0; edge_id < instance.number_of_edges(); ++edge_id) {
        VertexId vertex_id_1 = instance.edge(edge_id).vertex_id_1;
        VertexId vertex_id_2 = instance.edge(edge_id).vertex_id_2;
        VertexId vertex_id_1_new = original2reduced[vertex_id_1];
        VertexId vertex_id_2_new = original2reduced[vertex_id_2];
        if (vertex_id_1_new != -1 && vertex_id_2_new != -1) {
            reduction_output.instance->add_edge(vertex_id_1_new, vertex_id_2_new, 0);
        }
    }
    reduction_output.instance->compute_components();

    return reduction_output;
}

ReductionOutput Instance::reduce_isolated_vertex_removal(
        const ReductionOutput& reduction_output_old)
{
    //std::cout << "Isolated vertex removal..." << std::endl;
    const Instance& instance = *reduction_output_old.instance;
    optimizationtools::IndexedSet neighbors(instance.number_of_vertices());
    optimizationtools::DoublyIndexedMap fixed_vertices(instance.number_of_vertices(), 2);
    for (VertexId vertex_id = 0;
            vertex_id < instance.number_of_vertices();
            ++vertex_id) {
        if (fixed_vertices.contains(vertex_id))
            continue;
        bool neighbors_clique = true;
        for (const auto& edge: instance.vertex(vertex_id).edges) {
            if (fixed_vertices.contains(edge.vertex_id))
                continue;
            // Check if all neighbors of v are neighbors of edge.v.
            neighbors.clear();
            neighbors.add(edge.vertex_id);
            for (const auto& edge_2: instance.vertex(edge.vertex_id).edges)
                if (!fixed_vertices.contains(edge_2.vertex_id))
                    neighbors.add(edge_2.vertex_id);
            for (const auto& edge: instance.vertex(vertex_id).edges) {
                if (!fixed_vertices.contains(edge.vertex_id)) {
                    if (!neighbors.contains(edge.vertex_id)
                            || instance.vertex(vertex_id).weight
                            < instance.vertex(edge.vertex_id).weight) {
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
            for (const auto& edge: instance.vertex(vertex_id).edges) {
                fixed_vertices.set(edge.vertex_id, 0);
            }
        }
    }
    //std::cout << fixed_vertices.number_of_elements() << std::endl;

    ReductionOutput reduction_output;
    if (fixed_vertices.number_of_elements() == 0)
        return reduction_output;

    // Update mandatory_vertices.
    reduction_output.mandatory_vertices = reduction_output_old.mandatory_vertices;
    for (auto it = fixed_vertices.begin(1); it != fixed_vertices.end(1); ++it) {
        VertexId vertex_id = *it;
        for (VertexId vertex_id_2: reduction_output_old.unreduction_operations[vertex_id].in) {
            reduction_output.mandatory_vertices.push_back(vertex_id_2);
        }
    }
    for (auto it = fixed_vertices.begin(0); it != fixed_vertices.end(0); ++it) {
        VertexId vertex_id = *it;
        for (VertexId vertex_id_2: reduction_output_old.unreduction_operations[vertex_id].out) {
            reduction_output.mandatory_vertices.push_back(vertex_id_2);
        }
    }
    // Update instance and unreduction_operations.
    VertexId n = instance.number_of_vertices() - fixed_vertices.number_of_elements();
    reduction_output.instance = new Instance(n);
    reduction_output.unreduction_operations = std::vector<UnreductionOperations>(n);
    // Add vertices.
    std::vector<VertexId> original2reduced(instance.number_of_vertices(), -1);
    VertexId vertex_id_new = 0;
    for (auto it = fixed_vertices.out_begin(); it != fixed_vertices.out_end(); ++it) {
        VertexId vertex_id = *it;
        original2reduced[vertex_id] = vertex_id_new;
        reduction_output.instance->set_weight(vertex_id_new, instance.vertex(vertex_id).weight);
        reduction_output.unreduction_operations[vertex_id_new]
            = reduction_output_old.unreduction_operations[vertex_id];
        vertex_id_new++;
    }
    // Add edges.
    for (EdgeId edge_id = 0; edge_id < instance.number_of_edges(); ++edge_id) {
        VertexId vertex_id_1 = instance.edge(edge_id).vertex_id_1;
        VertexId vertex_id_2 = instance.edge(edge_id).vertex_id_2;
        VertexId vertex_id_1_new = original2reduced[vertex_id_1];
        VertexId vertex_id_2_new = original2reduced[vertex_id_2];
        if (vertex_id_1_new != -1 && vertex_id_2_new != -1) {
            reduction_output.instance->add_edge(vertex_id_1_new, vertex_id_2_new, 0);
        }
    }
    reduction_output.instance->compute_components();

    return reduction_output;
}

ReductionOutput Instance::reduce_vertex_folding(
        const ReductionOutput& reduction_output_old)
{
    //std::cout << "Vertex folding..." << std::endl;
    const Instance& instance = *reduction_output_old.instance;
    optimizationtools::IndexedSet folded_vertices(instance.number_of_vertices());
    std::vector<std::tuple<VertexId, VertexId, VertexId>> folded_vertices_list;
    for (VertexId vertex_id = 0;
            vertex_id < instance.number_of_vertices();
            ++vertex_id) {
        if (instance.degree(vertex_id) != 2)
            continue;
        VertexId vertex_id_1 = instance.vertex(vertex_id).edges[0].vertex_id;
        VertexId vertex_id_2 = instance.vertex(vertex_id).edges[1].vertex_id;
        if (folded_vertices.contains(vertex_id)
                || folded_vertices.contains(vertex_id_1)
                || folded_vertices.contains(vertex_id_2))
            continue;
        if (instance.vertex(vertex_id).weight != instance.vertex(vertex_id_1).weight
                || instance.vertex(vertex_id).weight != instance.vertex(vertex_id_2).weight)
            continue;
        // Check if there exists an edge (vertex_id_1, vertex_id_2).
        bool ok = true;
        for (const auto& edge: instance.vertex(vertex_id_1).edges) {
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

    ReductionOutput reduction_output;
    if (folded_vertices_list.empty())
        return reduction_output;

    // Update mandatory_vertices.
    reduction_output.mandatory_vertices = reduction_output_old.mandatory_vertices;
    // Update instance and unreduction_operations.
    VertexId n = instance.number_of_vertices() - folded_vertices.size() + folded_vertices_list.size();
    reduction_output.instance = new Instance(n);
    reduction_output.unreduction_operations = std::vector<UnreductionOperations>(n);
    // Add vertices.
    std::vector<VertexId> original2reduced(instance.number_of_vertices(), -1);
    VertexId vertex_id_new = 0;
    for (auto it = folded_vertices.out_begin(); it != folded_vertices.out_end(); ++it) {
        VertexId vertex_id = *it;
        original2reduced[vertex_id] = vertex_id_new;
        reduction_output.instance->set_weight(vertex_id_new, instance.vertex(vertex_id).weight);
        reduction_output.unreduction_operations[vertex_id_new]
            = reduction_output_old.unreduction_operations[vertex_id];
        vertex_id_new++;
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
        original2reduced[vertex_id] = vertex_id_new;
        original2reduced[vertex_id_1] = vertex_id_new;
        original2reduced[vertex_id_2] = vertex_id_new;
        reduction_output.instance->set_weight(vertex_id_new, instance.vertex(vertex_id).weight);

        for (VertexId vertex_id_3: reduction_output_old.unreduction_operations[vertex_id].out)
            reduction_output.unreduction_operations[vertex_id_new].in.push_back(vertex_id_3);
        for (VertexId vertex_id_3: reduction_output_old.unreduction_operations[vertex_id].in)
            reduction_output.unreduction_operations[vertex_id_new].out.push_back(vertex_id_3);

        for (VertexId vertex_id_3: reduction_output_old.unreduction_operations[vertex_id_1].in)
            reduction_output.unreduction_operations[vertex_id_new].in.push_back(vertex_id_3);
        for (VertexId vertex_id_3: reduction_output_old.unreduction_operations[vertex_id_1].out)
            reduction_output.unreduction_operations[vertex_id_new].out.push_back(vertex_id_3);

        for (VertexId vertex_id_3: reduction_output_old.unreduction_operations[vertex_id_2].in)
            reduction_output.unreduction_operations[vertex_id_new].in.push_back(vertex_id_3);
        for (VertexId vertex_id_3: reduction_output_old.unreduction_operations[vertex_id_2].out)
            reduction_output.unreduction_operations[vertex_id_new].out.push_back(vertex_id_3);

        vertex_id_new++;
    }
    // Add edges.
    for (EdgeId e = 0; e < instance.number_of_edges(); ++e) {
        VertexId vertex_id_1 = instance.edge(e).vertex_id_1;
        VertexId vertex_id_2 = instance.edge(e).vertex_id_2;
        if (folded_vertices.contains(vertex_id_1) || folded_vertices.contains(vertex_id_2))
            continue;
        VertexId vertex_id_1_new = original2reduced[vertex_id_1];
        VertexId vertex_id_2_new = original2reduced[vertex_id_2];
        reduction_output.instance->add_edge(vertex_id_1_new, vertex_id_2_new, 0);
    }
    optimizationtools::IndexedSet neighbors_tmp(n);
    for (const auto& tuple: folded_vertices_list) {
        VertexId vertex_id = std::get<0>(tuple);
        VertexId vertex_id_1 = std::get<1>(tuple);
        VertexId vertex_id_2 = std::get<2>(tuple);
        VertexId vertex_id_new = original2reduced[vertex_id];

        neighbors_tmp.clear();
        for (const auto& edge: instance.vertex(vertex_id_1).edges)
            if (edge.vertex_id != vertex_id)
                if (!folded_vertices.contains(edge.vertex_id)
                        || vertex_id_new < original2reduced[edge.vertex_id])
                    neighbors_tmp.add(original2reduced[edge.vertex_id]);
        for (const auto& edge: instance.vertex(vertex_id_2).edges)
            if (edge.vertex_id != vertex_id)
                if (!folded_vertices.contains(edge.vertex_id)
                        || vertex_id_new < original2reduced[edge.vertex_id])
                    neighbors_tmp.add(original2reduced[edge.vertex_id]);
        for (VertexId vertex_id_3_new: neighbors_tmp)
            reduction_output.instance->add_edge(vertex_id_new, vertex_id_3_new, 0);
    }
    reduction_output.instance->compute_components();

    return reduction_output;
}

ReductionOutput Instance::reduce_twin(
        const ReductionOutput& reduction_output_old)
{
    //std::cout << "Vertex folding..." << std::endl;
    const Instance& instance = *reduction_output_old.instance;
    // 0: removed
    // 1: added
    // 2: folded
    optimizationtools::DoublyIndexedMap modified_vertices(instance.number_of_vertices(), 3);
    optimizationtools::IndexedMap<VertexPos> twin_candidates(instance.number_of_vertices(), 0);
    std::vector<std::tuple<VertexId, VertexId, VertexId, VertexId, VertexId>> folded_vertices_list;
    for (VertexId vertex_id = 0;
            vertex_id < instance.number_of_vertices();
            ++vertex_id) {
        if (instance.degree(vertex_id) != 3)
            continue;
        VertexId vertex_id_1 = instance.vertex(vertex_id).edges[0].vertex_id;
        VertexId vertex_id_2 = instance.vertex(vertex_id).edges[1].vertex_id;
        VertexId vertex_id_3 = instance.vertex(vertex_id).edges[2].vertex_id;
        if (modified_vertices.contains(vertex_id)
                || modified_vertices.contains(vertex_id_1)
                || modified_vertices.contains(vertex_id_2)
                || modified_vertices.contains(vertex_id_3))
            continue;
        Weight weight = instance.vertex(vertex_id).weight;
        if (instance.vertex(vertex_id_1).weight != weight
                || instance.vertex(vertex_id_2).weight != weight
                || instance.vertex(vertex_id_3).weight != weight)
            continue;
        twin_candidates.clear();
        for (const auto& edge: instance.vertex(vertex_id).edges) {
            for (const auto& edge_2: instance.vertex(edge.vertex_id).edges) {
                if (instance.degree(edge_2.vertex_id) != 3)
                    continue;
                if (edge_2.vertex_id == vertex_id)
                    continue;
                if (instance.vertex(edge_2.vertex_id).weight != weight)
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
        for (const auto& edge: instance.vertex(vertex_id_1).edges)
            if (edge.vertex_id == vertex_id_2 || edge.vertex_id == vertex_id_3)
                has_edge = true;
        for (const auto& edge: instance.vertex(vertex_id_2).edges)
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

    ReductionOutput reduction_output;
    if (modified_vertices.number_of_elements() == 0)
        return reduction_output;

    // Update mandatory_vertices.
    reduction_output.mandatory_vertices = reduction_output_old.mandatory_vertices;
    for (auto it = modified_vertices.begin(1); it != modified_vertices.end(1); ++it) {
        VertexId vertex_id = *it;
        for (VertexId vertex_id_2: reduction_output_old.unreduction_operations[vertex_id].in) {
            reduction_output.mandatory_vertices.push_back(vertex_id_2);
        }
    }
    for (auto it = modified_vertices.begin(0); it != modified_vertices.end(0); ++it) {
        VertexId vertex_id = *it;
        for (VertexId vertex_id_2: reduction_output_old.unreduction_operations[vertex_id].out) {
            reduction_output.mandatory_vertices.push_back(vertex_id_2);
        }
    }
    // Update instance and unreduction_operations.
    VertexId n = instance.number_of_vertices()
        - modified_vertices.number_of_elements(0)
        - modified_vertices.number_of_elements(1)
        - modified_vertices.number_of_elements(2)
        + modified_vertices.number_of_elements(2) / 5;
    reduction_output.instance = new Instance(n);
    reduction_output.unreduction_operations = std::vector<UnreductionOperations>(n);
    // Add vertices.
    std::vector<VertexId> original2reduced(instance.number_of_vertices(), -1);
    VertexId vertex_id_new = 0;
    for (auto it = modified_vertices.out_begin(); it != modified_vertices.out_end(); ++it) {
        VertexId vertex_id = *it;
        original2reduced[vertex_id] = vertex_id_new;
        reduction_output.instance->set_weight(vertex_id_new, instance.vertex(vertex_id).weight);
        reduction_output.unreduction_operations[vertex_id_new]
            = reduction_output_old.unreduction_operations[vertex_id];
        vertex_id_new++;
    }
    for (const auto& tuple: folded_vertices_list) {
        VertexId vertex_id = std::get<0>(tuple);
        VertexId vertex_id_twin = std::get<1>(tuple);
        VertexId vertex_id_1 = std::get<2>(tuple);
        VertexId vertex_id_2 = std::get<3>(tuple);
        VertexId vertex_id_3 = std::get<4>(tuple);
        original2reduced[vertex_id] = vertex_id_new;
        original2reduced[vertex_id_twin] = vertex_id_new;
        original2reduced[vertex_id_1] = vertex_id_new;
        original2reduced[vertex_id_2] = vertex_id_new;
        original2reduced[vertex_id_3] = vertex_id_new;
        reduction_output.instance->set_weight(vertex_id_new, instance.vertex(vertex_id).weight);

        for (VertexId vertex_id_tmp: reduction_output_old.unreduction_operations[vertex_id].out)
            reduction_output.unreduction_operations[vertex_id_new].in.push_back(vertex_id_tmp);
        for (VertexId vertex_id_tmp: reduction_output_old.unreduction_operations[vertex_id].in)
            reduction_output.unreduction_operations[vertex_id_new].out.push_back(vertex_id_tmp);

        for (VertexId vertex_id_tmp: reduction_output_old.unreduction_operations[vertex_id_twin].out)
            reduction_output.unreduction_operations[vertex_id_new].in.push_back(vertex_id_tmp);
        for (VertexId vertex_id_tmp: reduction_output_old.unreduction_operations[vertex_id_twin].in)
            reduction_output.unreduction_operations[vertex_id_new].out.push_back(vertex_id_tmp);

        for (VertexId vertex_id_tmp: reduction_output_old.unreduction_operations[vertex_id_1].in)
            reduction_output.unreduction_operations[vertex_id_new].in.push_back(vertex_id_tmp);
        for (VertexId vertex_id_tmp: reduction_output_old.unreduction_operations[vertex_id_1].out)
            reduction_output.unreduction_operations[vertex_id_new].out.push_back(vertex_id_tmp);

        for (VertexId vertex_id_tmp: reduction_output_old.unreduction_operations[vertex_id_2].in)
            reduction_output.unreduction_operations[vertex_id_new].in.push_back(vertex_id_tmp);
        for (VertexId vertex_id_tmp: reduction_output_old.unreduction_operations[vertex_id_2].out)
            reduction_output.unreduction_operations[vertex_id_new].out.push_back(vertex_id_tmp);

        for (VertexId vertex_id_tmp: reduction_output_old.unreduction_operations[vertex_id_3].in)
            reduction_output.unreduction_operations[vertex_id_new].in.push_back(vertex_id_tmp);
        for (VertexId vertex_id_tmp: reduction_output_old.unreduction_operations[vertex_id_3].out)
            reduction_output.unreduction_operations[vertex_id_new].out.push_back(vertex_id_tmp);

        vertex_id_new++;
    }
    // Add edges.
    for (EdgeId edge_id = 0; edge_id < instance.number_of_edges(); ++edge_id) {
        VertexId vertex_id_1 = instance.edge(edge_id).vertex_id_1;
        VertexId vertex_id_2 = instance.edge(edge_id).vertex_id_2;
        if (modified_vertices.contains(vertex_id_1) || modified_vertices.contains(vertex_id_2))
            continue;
        VertexId vertex_id_1_new = original2reduced[vertex_id_1];
        VertexId vertex_id_2_new = original2reduced[vertex_id_2];
        reduction_output.instance->add_edge(vertex_id_1_new, vertex_id_2_new, 0);
    }
    optimizationtools::IndexedSet neighbors_tmp(n);
    for (const auto& tuple: folded_vertices_list) {
        VertexId vertex_id = std::get<0>(tuple);
        VertexId vertex_id_twin = std::get<1>(tuple);
        VertexId vertex_id_1 = std::get<2>(tuple);
        VertexId vertex_id_2 = std::get<3>(tuple);
        VertexId vertex_id_3 = std::get<4>(tuple);
        VertexId vertex_id_new = original2reduced[vertex_id];

        neighbors_tmp.clear();
        for (const auto& edge: instance.vertex(vertex_id_1).edges)
            if (edge.vertex_id != vertex_id
                    && edge.vertex_id != vertex_id_twin)
                if (!modified_vertices.contains(edge.vertex_id)
                        || vertex_id_new < original2reduced[edge.vertex_id])
                    neighbors_tmp.add(original2reduced[edge.vertex_id]);
        for (const auto& edge: instance.vertex(vertex_id_2).edges)
            if (edge.vertex_id != vertex_id
                    && edge.vertex_id != vertex_id_twin)
                if (!modified_vertices.contains(edge.vertex_id)
                        || vertex_id_new < original2reduced[edge.vertex_id])
                    neighbors_tmp.add(original2reduced[edge.vertex_id]);
        for (const auto& edge: instance.vertex(vertex_id_3).edges)
            if (edge.vertex_id != vertex_id
                    && edge.vertex_id != vertex_id_twin)
                if (!modified_vertices.contains(edge.vertex_id)
                        || vertex_id_new < original2reduced[edge.vertex_id])
                    neighbors_tmp.add(original2reduced[edge.vertex_id]);
        for (VertexId vertex_id_tmp: neighbors_tmp)
            reduction_output.instance->add_edge(vertex_id_new, vertex_id_tmp, 0);
    }
    reduction_output.instance->compute_components();

    return reduction_output;
}

ReductionOutput Instance::reduce_domination(
        const ReductionOutput& reduction_output_old)
{
    //std::cout << "Vertex folding..." << std::endl;
    const Instance& instance = *reduction_output_old.instance;
    optimizationtools::IndexedSet removed_vertices(instance.number_of_vertices());
    optimizationtools::IndexedSet neighbors(instance.number_of_vertices());
    for (VertexId vertex_id = 0;
            vertex_id < instance.number_of_vertices();
            ++vertex_id) {
        Weight weight = instance.vertex(vertex_id).weight;
        neighbors.clear();
        for (const auto& edge: instance.vertex(vertex_id).edges)
            neighbors.add(edge.vertex_id);
        bool can_be_removed = false;
        for (const auto& edge: instance.vertex(vertex_id).edges) {
            if (instance.vertex(edge.vertex_id).weight < weight)
                continue;
            if (removed_vertices.contains(edge.vertex_id))
                continue;
            bool dominates = true;
            for (const auto& edge_2: instance.vertex(edge.vertex_id).edges) {
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

    ReductionOutput reduction_output;
    if (removed_vertices.size() == 0)
        return reduction_output;

    // Update mandatory_vertices.
    reduction_output.mandatory_vertices = reduction_output_old.mandatory_vertices;
    for (VertexId vertex_id: removed_vertices) {
        for (VertexId vertex_id_2: reduction_output_old.unreduction_operations[vertex_id].out) {
            reduction_output.mandatory_vertices.push_back(vertex_id_2);
        }
    }
    // Update instance and unreduction_operations.
    VertexId n = instance.number_of_vertices() - removed_vertices.size();
    reduction_output.instance = new Instance(n);
    reduction_output.unreduction_operations = std::vector<UnreductionOperations>(n);
    // Add vertices.
    std::vector<VertexId> original2reduced(instance.number_of_vertices(), -1);
    VertexId vertex_id_new = 0;
    for (auto it = removed_vertices.out_begin(); it != removed_vertices.out_end(); ++it) {
        VertexId vertex_id = *it;
        original2reduced[vertex_id] = vertex_id_new;
        reduction_output.instance->set_weight(vertex_id_new, instance.vertex(vertex_id).weight);
        reduction_output.unreduction_operations[vertex_id_new]
            = reduction_output_old.unreduction_operations[vertex_id];
        vertex_id_new++;
    }
    // Add edges.
    for (EdgeId edge_id = 0; edge_id < instance.number_of_edges(); ++edge_id) {
        VertexId vertex_id_1 = instance.edge(edge_id).vertex_id_1;
        VertexId vertex_id_2 = instance.edge(edge_id).vertex_id_2;
        if (removed_vertices.contains(vertex_id_1) || removed_vertices.contains(vertex_id_2))
            continue;
        VertexId vertex_id_1_new = original2reduced[vertex_id_1];
        VertexId vertex_id_2_new = original2reduced[vertex_id_2];
        reduction_output.instance->add_edge(vertex_id_1_new, vertex_id_2_new, 0);
    }
    reduction_output.instance->compute_components();

    return reduction_output;
}

ReductionOutput Instance::reduce_unconfined(
        const ReductionOutput& reduction_output_old)
{
    //std::cout << "Vertex folding..." << std::endl;
    const Instance& instance = *reduction_output_old.instance;
    optimizationtools::IndexedSet removed_vertices(instance.number_of_vertices());
    optimizationtools::IndexedSet s(instance.number_of_vertices());
    optimizationtools::IndexedSet n_s(instance.number_of_vertices());
    optimizationtools::IndexedSet neighbors(instance.number_of_vertices());
    for (VertexId vertex_id = 0;
            vertex_id < instance.number_of_vertices();
            ++vertex_id) {
        // Minimum weight in S.
        // The unconfined reduction rule remains true while the minimum weight
        // in S is greater or equal to the maximum weight in N(S).
        Weight ws_min = instance.vertex(vertex_id).weight;
        Weight wns_max = 0;

        s.clear();
        n_s.clear();

        // Update S.
        s.add(vertex_id);
        // Update N(S).
        for (const auto& edge: instance.vertex(vertex_id).edges) {
            n_s.add(edge.vertex_id);
            if (wns_max < instance.vertex(edge.vertex_id).weight)
                wns_max = instance.vertex(edge.vertex_id).weight;
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
                for (const auto& edge: instance.vertex(u).edges)
                    if (s.contains(edge.vertex_id))
                        n_u_inter_s_card++;
                if (n_u_inter_s_card != 1)
                    continue;

                VertexPos n_u_minus_n_s_card = 0;
                VertexPos w = -1;
                for (const auto& edge: instance.vertex(u).edges) {
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
                if (ws_min > instance.vertex(w_best).weight)
                    ws_min = instance.vertex(w_best).weight;
                // Update N(S).
                if (n_s.contains(w_best))
                    n_s.remove(w_best);
                for (const auto& edge: instance.vertex(w_best).edges) {
                    if (!s.contains(edge.vertex_id)) {
                        n_s.add(edge.vertex_id);
                        if (wns_max < instance.vertex(edge.vertex_id).weight)
                            wns_max = instance.vertex(edge.vertex_id).weight;
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

    ReductionOutput reduction_output;
    if (removed_vertices.size() == 0)
        return reduction_output;

    // Update mandatory_vertices.
    reduction_output.mandatory_vertices = reduction_output_old.mandatory_vertices;
    for (VertexId vertex_id: removed_vertices) {
        for (VertexId vertex_id_2: reduction_output_old.unreduction_operations[vertex_id].out) {
            reduction_output.mandatory_vertices.push_back(vertex_id_2);
        }
    }
    // Update instance and unreduction_operations.
    VertexId n = instance.number_of_vertices() - removed_vertices.size();
    reduction_output.instance = new Instance(n);
    reduction_output.unreduction_operations = std::vector<UnreductionOperations>(n);
    // Add vertices.
    std::vector<VertexId> original2reduced(instance.number_of_vertices(), -1);
    VertexId vertex_id_new = 0;
    for (auto it = removed_vertices.out_begin(); it != removed_vertices.out_end(); ++it) {
        VertexId vertex_id = *it;
        original2reduced[vertex_id] = vertex_id_new;
        reduction_output.instance->set_weight(
                vertex_id_new,
                instance.vertex(vertex_id).weight);
        reduction_output.unreduction_operations[vertex_id_new]
            = reduction_output_old.unreduction_operations[vertex_id];
        vertex_id_new++;
    }
    // Add edges.
    for (EdgeId edge_id = 0; edge_id < instance.number_of_edges(); ++edge_id) {
        VertexId vertex_id_1 = instance.edge(edge_id).vertex_id_1;
        VertexId vertex_id_2 = instance.edge(edge_id).vertex_id_2;
        if (removed_vertices.contains(vertex_id_1)
                || removed_vertices.contains(vertex_id_2))
            continue;
        VertexId vertex_id_1_new = original2reduced[vertex_id_1];
        VertexId vertex_id_2_new = original2reduced[vertex_id_2];
        reduction_output.instance->add_edge(vertex_id_1_new, vertex_id_2_new, 0);
    }
    reduction_output.instance->compute_components();

    return reduction_output;
}

void Instance::reduce()
{
    // Compute fixed vertices.
    reduction_output_.instance = this;
    reduction_output_.unreduction_operations
        = std::vector<UnreductionOperations>(number_of_vertices());
    for (VertexId vertex_id = 0;
            vertex_id < number_of_vertices();
            ++vertex_id) {
        reduction_output_.unreduction_operations[vertex_id].in.push_back(vertex_id);
    }

    for (Counter round_number = 0; round_number < 10; ++round_number) {
        bool found = false;

        {
            auto reduction_output = reduce_pendant_vertices(reduction_output_);
            if (reduction_output.instance != nullptr) {
                found = true;
                if (reduction_output_.instance != this)
                    delete reduction_output_.instance;
                reduction_output_ = reduction_output;
            }
        }

        {
            auto reduction_output = reduce_vertex_folding(reduction_output_);
            if (reduction_output.instance != nullptr) {
                found = true;
                if (reduction_output_.instance != this)
                    delete reduction_output_.instance;
                reduction_output_ = reduction_output;
            }
        }

        {
            auto reduction_output = reduce_isolated_vertex_removal(reduction_output_);
            if (reduction_output.instance != nullptr) {
                found = true;
                if (reduction_output_.instance != this)
                    delete reduction_output_.instance;
                reduction_output_ = reduction_output;
            }
        }

        {
            auto reduction_output = reduce_twin(reduction_output_);
            if (reduction_output.instance != nullptr) {
                found = true;
                if (reduction_output_.instance != this)
                    delete reduction_output_.instance;
                reduction_output_ = reduction_output;
            }
        }

        {
            auto reduction_output = reduce_domination(reduction_output_);
            if (reduction_output.instance != nullptr) {
                found = true;
                if (reduction_output_.instance != this)
                    delete reduction_output_.instance;
                reduction_output_ = reduction_output;
            }
        }

        {
            auto reduction_output = reduce_unconfined(reduction_output_);
            if (reduction_output.instance != nullptr) {
                found = true;
                if (reduction_output_.instance != this)
                    delete reduction_output_.instance;
                reduction_output_ = reduction_output;
            }
        }

        if (!found)
            break;
    }

    extra_weight_ = 0;
    for (VertexId vertex_id: reduction_output_.mandatory_vertices)
        extra_weight_ += vertex(vertex_id).weight;
    for (VertexId vertex_id = 0;
            vertex_id < reduction_output_.instance->number_of_vertices();
            ++vertex_id) {
        extra_weight_ += reduction_output_.unreduction_operations[vertex_id].out.size();
    }
}

void stablesolver::init_display(
        const Instance& instance,
        optimizationtools::Info& info)
{
    VertexId n = instance.number_of_vertices();
    EdgeId m = instance.number_of_edges();
    info.os()
        << "=====================================" << std::endl
        << "            Stable Solver            " << std::endl
        << "=====================================" << std::endl
        << std::endl
        << "Instance" << std::endl
        << "--------" << std::endl
        << "Number of vertices:              " << n << std::endl
        << "Number of edges:                 " << m << std::endl
        << "Density:                         " << (double)m * 2 / n / (n - 1) << std::endl
        << "Average degree:                  " << (double)instance.number_of_edges() * 2 / n << std::endl
        << "Maximum degree:                  " << instance.maximum_degree() << std::endl
        << "Average weight:                  " << (double)instance.total_weight() / n << std::endl
        << "Number of connected components:  " << instance.number_of_components() << std::endl
        << std::endl;

    const Instance* reduced_instance = instance.reduced_instance();
    if (reduced_instance != nullptr) {
        VertexId n = reduced_instance->number_of_vertices();
        EdgeId m = reduced_instance->number_of_edges();
        info.os()
            << "Reduced instance" << std::endl
            << "----------------" << std::endl
            << "Number of vertices:              " << n << std::endl
            << "Number of edges:                 " << m << std::endl
            << "Density:                         " << (double)m * 2 / n / (n - 1) << std::endl
            << "Average degree:                  " << (double)reduced_instance->number_of_edges() * 2 / n << std::endl
            << "Maximum degree:                  " << reduced_instance->maximum_degree() << std::endl
            << "Extra weight:                    " << instance.extra_weight() << std::endl
            << "Number of connected components:  " << reduced_instance->number_of_components() << std::endl
            << std::endl;
    }
}

