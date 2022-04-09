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
    if (!file.good())
        throw std::runtime_error(
                "Unable to open file \"" + instance_path + "\".");

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
    for (VertexId v = 0; v < number_of_vertices; ++v)
        add_vertex();
}

void Instance::add_vertex(Weight weight)
{
    Vertex vertex;
    vertex.id = vertices_.size();
    vertex.weight = weight;
    total_weight_ += weight;
    vertices_.push_back(vertex);
}

void Instance::add_edge(VertexId v1, VertexId v2, int check_duplicate)
{
    if (v1 == v2) {
        std::cerr << "\033[33m" << "WARNING, loop (" << v1 << ", " << v2 << ") ignored." << "\033[0m" << std::endl;
        return;
    }

    if (check_duplicate > 0) {
        for (const auto& edge: vertex(v1).edges) {
            if (edge.v == v2) {
                if (check_duplicate == 1) {
                    return;
                } else {
                    throw std::runtime_error(
                            "Duplicate edge: ("
                            + std::to_string(v1)
                            + ","
                            + std::to_string(v2)
                            + ").");
                }
            }
        }
    }

    Edge e;
    e.id = edges_.size();
    e.v1 = v1;
    e.v2 = v2;
    edges_.push_back(e);

    VertexEdge ve1;
    ve1.e = e.id;
    ve1.v = v2;
    vertices_[v1].edges.push_back(ve1);
    if (maximum_degree_ < (VertexId)vertices_[v1].edges.size())
        maximum_degree_ = vertices_[v1].edges.size();

    VertexEdge ve2;
    ve2.e = e.id;
    ve2.v = v1;
    vertices_[v2].edges.push_back(ve2);
    if (maximum_degree_ < (VertexId)vertices_[v2].edges.size())
        maximum_degree_ = vertices_[v2].edges.size();
}

void Instance::set_weight(VertexId v, Weight w)
{
    if (w < 0) {
        throw std::invalid_argument("Set negative weight '"
                + std::to_string(w)
                + "' to vertex '"
                + std::to_string(v)
                + "'.");
    }
    total_weight_ -= vertex(v).weight;
    vertices_[v].weight = w;
    total_weight_ += vertex(v).weight;
}

void Instance::set_unweighted()
{
    for (VertexId v = 0; v < number_of_vertices(); ++v)
        vertices_[v].weight = 1;
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
            for (VertexId v = 0; v < number_of_vertices; ++v)
                vertices_[v].id = v;
            total_weight_ = number_of_vertices;
        } else if (line[0] == "n") {
            VertexId v = stol(line[1]) - 1;
            Weight w = stol(line[2]);
            set_weight(v, w);
        } else if (line[0] == "e") {
            VertexId v1 = stol(line[1]) - 1;
            VertexId v2 = stol(line[2]) - 1;
            add_edge(v1, v2);
        }
    }
}

void Instance::read_dimacs2010(std::ifstream& file)
{
    std::string tmp;
    std::vector<std::string> line;
    bool first = true;
    VertexId v = -1;
    while (v != number_of_vertices()) {
        getline(file, tmp);
        //std::cout << tmp << std::endl;
        line = optimizationtools::split(tmp, ' ');
        if (tmp[0] == '%')
            continue;
        if (first) {
            VertexId number_of_vertices = stol(line[0]);
            vertices_.resize(number_of_vertices);
            for (VertexId v = 0; v < number_of_vertices; ++v)
                vertices_[v].id = v;
            total_weight_ = number_of_vertices;
            first = false;
            v = 0;
        } else {
            for (std::string str: line) {
                VertexId v2 = stol(str) - 1;
                if (v2 > v)
                    add_edge(v, v2);
            }
            v++;
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
        VertexId v1 = stol(line[0]) - 1;
        VertexId v2 = stol(line[1]) - 1;
        add_edge(v1, v2);
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
            VertexId v2 = stol(str) - 1;
            if (v2 > v)
                add_edge(v, v2);
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

    VertexId v1 = -1;
    VertexId v2 = -1;
    for (;;) {
        file >> v1 >> v2;
        if (file.eof())
            break;
        while (std::max(v1, v2) >= number_of_vertices())
            add_vertex();
        add_edge(v1, v2);
    }
}

Instance Instance::complementary()
{
    Instance instance(number_of_vertices());
    optimizationtools::IndexedSet neighbors(number_of_vertices());

    for (VertexId v = 0; v < number_of_vertices(); ++v){
        instance.set_weight(v, vertex(v).weight);
        neighbors.clear();
        neighbors.add(v);
        for (const auto& edge: vertex(v).edges)
            neighbors.add(edge.v);
        for (auto it = neighbors.out_begin(); it != neighbors.out_end(); ++it)
            if (*it > v)
                instance.add_edge(v, *it);
    }

    instance.compute_components();
    return instance;
}

void Instance::compute_components()
{
    //std::cout << "compute_components" << std::endl;
    std::vector<VertexId> stack;
    VertexId v0 = 0;
    for (ComponentId c = 0;; ++c) {
        while (v0 < number_of_vertices()
                && (vertex(v0).component != -1))
            v0++;
        if (v0 == number_of_vertices())
            break;
        //std::cout << "c " << c << " v " << v << std::endl;
        Component component;
        component.id = c;
        stack.clear();
        stack.push_back(v0);
        vertices_[v0].component = c;
        while (!stack.empty()) {
            VertexId v = stack.back();
            stack.pop_back();
            for (const auto& edge: vertex(v).edges) {
                edges_[edge.e].component = c;
                if (vertex(edge.v).component != -1)
                    continue;
                vertices_[edge.v].component = c;
                stack.push_back(edge.v);
            }
        }
        components_.push_back(component);
    }

    for (VertexId v = 0; v < number_of_vertices(); ++v)
        if (vertex(v).component != -1)
            components_[vertex(v).component].vertices.push_back(v);
    for (EdgeId e = 0; e < number_of_edges(); ++e)
        components_[edge(e).component].edges.push_back(e);

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
    //    //    std::cout << " " << edge(e).v1 << "," << edge(e).v2;
    //    //std::cout << std::endl;
    //}
}

ReductionOutput Instance::reduce_pendant_vertices(
        const ReductionOutput& reduction_output_old)
{
    const Instance& instance = *reduction_output_old.instance;
    optimizationtools::IndexedSet neighbors(instance.number_of_vertices());
    optimizationtools::DoublyIndexedMap fixed_vertices(instance.number_of_vertices(), 2);
    for (VertexId v = 0; v < instance.number_of_vertices(); ++v) {
        if (instance.degree(v) != 1)
            continue;
        Weight w = instance.vertex(v).weight;
        VertexId v1 = instance.vertex(v).edges[0].v;
        if (instance.vertex(v1).weight > w)
            continue;
        fixed_vertices.set(v, 1);
        fixed_vertices.set(instance.vertex(v).edges[0].v, 0);
    }
    //std::cout << fixed_vertices.number_of_elements() << std::endl;

    ReductionOutput reduction_output;
    if (fixed_vertices.number_of_elements() == 0)
        return reduction_output;

    // Update mandatory_vertices.
    reduction_output.mandatory_vertices = reduction_output_old.mandatory_vertices;
    for (auto it = fixed_vertices.begin(1); it != fixed_vertices.end(1); ++it) {
        VertexId v = *it;
        for (VertexId v2: reduction_output_old.unreduction_operations[v].in) {
            reduction_output.mandatory_vertices.push_back(v2);
        }
    }
    for (auto it = fixed_vertices.begin(0); it != fixed_vertices.end(0); ++it) {
        VertexId v = *it;
        for (VertexId v2: reduction_output_old.unreduction_operations[v].out) {
            reduction_output.mandatory_vertices.push_back(v2);
        }
    }
    // Update instance and unreduction_operations.
    VertexId n = instance.number_of_vertices() - fixed_vertices.number_of_elements();
    reduction_output.instance = new Instance(n);
    reduction_output.unreduction_operations = std::vector<UnreductionOperations>(n);
    // Add vertices.
    std::vector<VertexId> original2reduced(instance.number_of_vertices(), -1);
    VertexId v_new = 0;
    for (auto it = fixed_vertices.out_begin(); it != fixed_vertices.out_end(); ++it) {
        VertexId v = *it;
        original2reduced[v] = v_new;
        reduction_output.instance->set_weight(v_new, instance.vertex(v).weight);
        reduction_output.unreduction_operations[v_new]
            = reduction_output_old.unreduction_operations[v];
        v_new++;
    }
    // Add edges.
    for (EdgeId e = 0; e < instance.number_of_edges(); ++e) {
        VertexId v1 = instance.edge(e).v1;
        VertexId v2 = instance.edge(e).v2;
        VertexId v1_new = original2reduced[v1];
        VertexId v2_new = original2reduced[v2];
        if (v1_new != -1 && v2_new != -1) {
            reduction_output.instance->add_edge(v1_new, v2_new, 0);
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
    for (VertexId v = 0; v < instance.number_of_vertices(); ++v) {
        if (fixed_vertices.contains(v))
            continue;
        bool neighbors_clique = true;
        for (const auto& edge: instance.vertex(v).edges) {
            if (fixed_vertices.contains(edge.v))
                continue;
            // Check if all neighbors of v are neighbors of edge.v.
            neighbors.clear();
            neighbors.add(edge.v);
            for (const auto& edge_2: instance.vertex(edge.v).edges)
                if (!fixed_vertices.contains(edge_2.v))
                    neighbors.add(edge_2.v);
            for (const auto& edge: instance.vertex(v).edges) {
                if (!fixed_vertices.contains(edge.v)) {
                    if (!neighbors.contains(edge.v)
                            || instance.vertex(v).weight < instance.vertex(edge.v).weight) {
                        neighbors_clique = false;
                        break;
                    }
                }
            }
            if (!neighbors_clique)
                break;
        }
        if (neighbors_clique) {
            fixed_vertices.set(v, 1);
            for (const auto& edge: instance.vertex(v).edges) {
                fixed_vertices.set(edge.v, 0);
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
        VertexId v = *it;
        for (VertexId v2: reduction_output_old.unreduction_operations[v].in) {
            reduction_output.mandatory_vertices.push_back(v2);
        }
    }
    for (auto it = fixed_vertices.begin(0); it != fixed_vertices.end(0); ++it) {
        VertexId v = *it;
        for (VertexId v2: reduction_output_old.unreduction_operations[v].out) {
            reduction_output.mandatory_vertices.push_back(v2);
        }
    }
    // Update instance and unreduction_operations.
    VertexId n = instance.number_of_vertices() - fixed_vertices.number_of_elements();
    reduction_output.instance = new Instance(n);
    reduction_output.unreduction_operations = std::vector<UnreductionOperations>(n);
    // Add vertices.
    std::vector<VertexId> original2reduced(instance.number_of_vertices(), -1);
    VertexId v_new = 0;
    for (auto it = fixed_vertices.out_begin(); it != fixed_vertices.out_end(); ++it) {
        VertexId v = *it;
        original2reduced[v] = v_new;
        reduction_output.instance->set_weight(v_new, instance.vertex(v).weight);
        reduction_output.unreduction_operations[v_new]
            = reduction_output_old.unreduction_operations[v];
        v_new++;
    }
    // Add edges.
    for (EdgeId e = 0; e < instance.number_of_edges(); ++e) {
        VertexId v1 = instance.edge(e).v1;
        VertexId v2 = instance.edge(e).v2;
        VertexId v1_new = original2reduced[v1];
        VertexId v2_new = original2reduced[v2];
        if (v1_new != -1 && v2_new != -1) {
            reduction_output.instance->add_edge(v1_new, v2_new, 0);
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
    for (VertexId v = 0; v < instance.number_of_vertices(); ++v) {
        if (instance.degree(v) != 2)
            continue;
        VertexId v1 = instance.vertex(v).edges[0].v;
        VertexId v2 = instance.vertex(v).edges[1].v;
        if (folded_vertices.contains(v)
                || folded_vertices.contains(v1)
                || folded_vertices.contains(v2))
            continue;
        if (instance.vertex(v).weight != instance.vertex(v1).weight
                || instance.vertex(v).weight != instance.vertex(v2).weight)
            continue;
        // Check if there exists an edge (v1, v2).
        bool ok = true;
        for (const auto& edge: instance.vertex(v1).edges) {
            if (edge.v == v2) {
                ok = false;
                break;
            }
        }
        if (!ok)
            continue;
        //std::cout << "v " << v << " v1 " << v1 << " v2 " << v2 << std::endl;
        folded_vertices.add(v);
        folded_vertices.add(v1);
        folded_vertices.add(v2);
        folded_vertices_list.push_back({v, v1, v2});
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
    VertexId v_new = 0;
    for (auto it = folded_vertices.out_begin(); it != folded_vertices.out_end(); ++it) {
        VertexId v = *it;
        original2reduced[v] = v_new;
        reduction_output.instance->set_weight(v_new, instance.vertex(v).weight);
        reduction_output.unreduction_operations[v_new]
            = reduction_output_old.unreduction_operations[v];
        v_new++;
    }
    for (const auto& tuple: folded_vertices_list) {
        VertexId v = std::get<0>(tuple);
        VertexId v1 = std::get<1>(tuple);
        VertexId v2 = std::get<2>(tuple);
        if (v1 == v2) {
            throw std::runtime_error(
                    "Vertex "
                    + std::to_string(v)
                    + " has two times vertex "
                    + std::to_string(v1)
                    + " as neighbor.");
        }
        original2reduced[v] = v_new;
        original2reduced[v1] = v_new;
        original2reduced[v2] = v_new;
        reduction_output.instance->set_weight(v_new, instance.vertex(v).weight);

        for (VertexId v3: reduction_output_old.unreduction_operations[v].out)
            reduction_output.unreduction_operations[v_new].in.push_back(v3);
        for (VertexId v3: reduction_output_old.unreduction_operations[v].in)
            reduction_output.unreduction_operations[v_new].out.push_back(v3);

        for (VertexId v3: reduction_output_old.unreduction_operations[v1].in)
            reduction_output.unreduction_operations[v_new].in.push_back(v3);
        for (VertexId v3: reduction_output_old.unreduction_operations[v1].out)
            reduction_output.unreduction_operations[v_new].out.push_back(v3);

        for (VertexId v3: reduction_output_old.unreduction_operations[v2].in)
            reduction_output.unreduction_operations[v_new].in.push_back(v3);
        for (VertexId v3: reduction_output_old.unreduction_operations[v2].out)
            reduction_output.unreduction_operations[v_new].out.push_back(v3);

        v_new++;
    }
    // Add edges.
    for (EdgeId e = 0; e < instance.number_of_edges(); ++e) {
        VertexId v1 = instance.edge(e).v1;
        VertexId v2 = instance.edge(e).v2;
        if (folded_vertices.contains(v1) || folded_vertices.contains(v2))
            continue;
        VertexId v1_new = original2reduced[v1];
        VertexId v2_new = original2reduced[v2];
        reduction_output.instance->add_edge(v1_new, v2_new, 0);
    }
    optimizationtools::IndexedSet neighbors_tmp(n);
    for (const auto& tuple: folded_vertices_list) {
        VertexId v = std::get<0>(tuple);
        VertexId v1 = std::get<1>(tuple);
        VertexId v2 = std::get<2>(tuple);
        VertexId v_new = original2reduced[v];

        neighbors_tmp.clear();
        for (const auto& edge: instance.vertex(v1).edges)
            if (edge.v != v)
                if (!folded_vertices.contains(edge.v)
                        || v_new < original2reduced[edge.v])
                    neighbors_tmp.add(original2reduced[edge.v]);
        for (const auto& edge: instance.vertex(v2).edges)
            if (edge.v != v)
                if (!folded_vertices.contains(edge.v)
                        || v_new < original2reduced[edge.v])
                    neighbors_tmp.add(original2reduced[edge.v]);
        for (VertexId v3_new: neighbors_tmp)
            reduction_output.instance->add_edge(v_new, v3_new, 0);
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
    for (VertexId v = 0; v < instance.number_of_vertices(); ++v) {
        if (instance.degree(v) != 3)
            continue;
        VertexId v1 = instance.vertex(v).edges[0].v;
        VertexId v2 = instance.vertex(v).edges[1].v;
        VertexId v3 = instance.vertex(v).edges[2].v;
        if (modified_vertices.contains(v)
                || modified_vertices.contains(v1)
                || modified_vertices.contains(v2)
                || modified_vertices.contains(v3))
            continue;
        Weight w = instance.vertex(v).weight;
        if (instance.vertex(v1).weight != w
                || instance.vertex(v2).weight != w
                || instance.vertex(v3).weight != w)
            continue;
        twin_candidates.clear();
        for (const auto& edge: instance.vertex(v).edges) {
            for (const auto& edge_2: instance.vertex(edge.v).edges) {
                if (instance.degree(edge_2.v) != 3)
                    continue;
                if (edge_2.v == v)
                    continue;
                if (instance.vertex(edge_2.v).weight != w)
                    continue;
                if (modified_vertices.contains(edge_2.v))
                    continue;
                twin_candidates.set(edge_2.v, twin_candidates[edge_2.v] + 1);
            }
        }
        VertexId v_twin = -1;
        for (auto p: twin_candidates) {
            if (p.second == 3) {
                v_twin = p.first;
                break;
            }
        }
        if (v_twin == -1)
            continue;
        // Is there an edge inside v1, v2, v3?
        bool has_edge = false;
        for (const auto& edge: instance.vertex(v1).edges)
            if (edge.v == v2 || edge.v == v3)
                has_edge = true;
        for (const auto& edge: instance.vertex(v2).edges)
            if (edge.v == v3)
                has_edge = true;

        if (has_edge) {
            modified_vertices.set(v, 1);
            modified_vertices.set(v_twin, 1);
            modified_vertices.set(v1, 0);
            modified_vertices.set(v2, 0);
            modified_vertices.set(v3, 0);
        } else {
            modified_vertices.set(v, 2);
            modified_vertices.set(v_twin, 2);
            modified_vertices.set(v1, 2);
            modified_vertices.set(v2, 2);
            modified_vertices.set(v3, 2);
            folded_vertices_list.push_back({v, v_twin, v1, v2, v3});
        }
    }
    //std::cout << folded_vertices.number_of_elements() << std::endl;

    ReductionOutput reduction_output;
    if (modified_vertices.number_of_elements() == 0)
        return reduction_output;

    // Update mandatory_vertices.
    reduction_output.mandatory_vertices = reduction_output_old.mandatory_vertices;
    for (auto it = modified_vertices.begin(1); it != modified_vertices.end(1); ++it) {
        VertexId v = *it;
        for (VertexId v2: reduction_output_old.unreduction_operations[v].in) {
            reduction_output.mandatory_vertices.push_back(v2);
        }
    }
    for (auto it = modified_vertices.begin(0); it != modified_vertices.end(0); ++it) {
        VertexId v = *it;
        for (VertexId v2: reduction_output_old.unreduction_operations[v].out) {
            reduction_output.mandatory_vertices.push_back(v2);
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
    VertexId v_new = 0;
    for (auto it = modified_vertices.out_begin(); it != modified_vertices.out_end(); ++it) {
        VertexId v = *it;
        original2reduced[v] = v_new;
        reduction_output.instance->set_weight(v_new, instance.vertex(v).weight);
        reduction_output.unreduction_operations[v_new]
            = reduction_output_old.unreduction_operations[v];
        v_new++;
    }
    for (const auto& tuple: folded_vertices_list) {
        VertexId v = std::get<0>(tuple);
        VertexId v_twin = std::get<1>(tuple);
        VertexId v1 = std::get<2>(tuple);
        VertexId v2 = std::get<3>(tuple);
        VertexId v3 = std::get<4>(tuple);
        original2reduced[v] = v_new;
        original2reduced[v_twin] = v_new;
        original2reduced[v1] = v_new;
        original2reduced[v2] = v_new;
        original2reduced[v3] = v_new;
        reduction_output.instance->set_weight(v_new, instance.vertex(v).weight);

        for (VertexId v_tmp: reduction_output_old.unreduction_operations[v].out)
            reduction_output.unreduction_operations[v_new].in.push_back(v_tmp);
        for (VertexId v_tmp: reduction_output_old.unreduction_operations[v].in)
            reduction_output.unreduction_operations[v_new].out.push_back(v_tmp);

        for (VertexId v_tmp: reduction_output_old.unreduction_operations[v_twin].out)
            reduction_output.unreduction_operations[v_new].in.push_back(v_tmp);
        for (VertexId v_tmp: reduction_output_old.unreduction_operations[v_twin].in)
            reduction_output.unreduction_operations[v_new].out.push_back(v_tmp);

        for (VertexId v_tmp: reduction_output_old.unreduction_operations[v1].in)
            reduction_output.unreduction_operations[v_new].in.push_back(v_tmp);
        for (VertexId v_tmp: reduction_output_old.unreduction_operations[v1].out)
            reduction_output.unreduction_operations[v_new].out.push_back(v_tmp);

        for (VertexId v_tmp: reduction_output_old.unreduction_operations[v2].in)
            reduction_output.unreduction_operations[v_new].in.push_back(v_tmp);
        for (VertexId v_tmp: reduction_output_old.unreduction_operations[v2].out)
            reduction_output.unreduction_operations[v_new].out.push_back(v_tmp);

        for (VertexId v_tmp: reduction_output_old.unreduction_operations[v3].in)
            reduction_output.unreduction_operations[v_new].in.push_back(v_tmp);
        for (VertexId v_tmp: reduction_output_old.unreduction_operations[v3].out)
            reduction_output.unreduction_operations[v_new].out.push_back(v_tmp);

        v_new++;
    }
    // Add edges.
    for (EdgeId e = 0; e < instance.number_of_edges(); ++e) {
        VertexId v1 = instance.edge(e).v1;
        VertexId v2 = instance.edge(e).v2;
        if (modified_vertices.contains(v1) || modified_vertices.contains(v2))
            continue;
        VertexId v1_new = original2reduced[v1];
        VertexId v2_new = original2reduced[v2];
        reduction_output.instance->add_edge(v1_new, v2_new, 0);
    }
    optimizationtools::IndexedSet neighbors_tmp(n);
    for (const auto& tuple: folded_vertices_list) {
        VertexId v = std::get<0>(tuple);
        VertexId v_twin = std::get<1>(tuple);
        VertexId v1 = std::get<2>(tuple);
        VertexId v2 = std::get<3>(tuple);
        VertexId v3 = std::get<4>(tuple);
        VertexId v_new = original2reduced[v];

        neighbors_tmp.clear();
        for (const auto& edge: instance.vertex(v1).edges)
            if (edge.v != v && edge.v != v_twin)
                if (!modified_vertices.contains(edge.v)
                        || v_new < original2reduced[edge.v])
                    neighbors_tmp.add(original2reduced[edge.v]);
        for (const auto& edge: instance.vertex(v2).edges)
            if (edge.v != v && edge.v != v_twin)
                if (!modified_vertices.contains(edge.v)
                        || v_new < original2reduced[edge.v])
                    neighbors_tmp.add(original2reduced[edge.v]);
        for (const auto& edge: instance.vertex(v3).edges)
            if (edge.v != v && edge.v != v_twin)
                if (!modified_vertices.contains(edge.v)
                        || v_new < original2reduced[edge.v])
                    neighbors_tmp.add(original2reduced[edge.v]);
        for (VertexId v_tmp: neighbors_tmp)
            reduction_output.instance->add_edge(v_new, v_tmp, 0);
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
    for (VertexId v = 0; v < instance.number_of_vertices(); ++v) {
        Weight w = instance.vertex(v).weight;
        neighbors.clear();
        for (const auto& edge: instance.vertex(v).edges)
            neighbors.add(edge.v);
        bool can_be_removed = false;
        for (const auto& edge: instance.vertex(v).edges) {
            if (instance.vertex(edge.v).weight < w)
                continue;
            if (removed_vertices.contains(edge.v))
                continue;
            bool dominates = true;
            for (const auto& edge_2: instance.vertex(edge.v).edges) {
                if (edge_2.v != v && !neighbors.contains(edge_2.v)) {
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
            removed_vertices.add(v);
    }
    //std::cout << folded_vertices.number_of_elements() << std::endl;

    ReductionOutput reduction_output;
    if (removed_vertices.size() == 0)
        return reduction_output;

    // Update mandatory_vertices.
    reduction_output.mandatory_vertices = reduction_output_old.mandatory_vertices;
    for (VertexId v: removed_vertices) {
        for (VertexId v2: reduction_output_old.unreduction_operations[v].out) {
            reduction_output.mandatory_vertices.push_back(v2);
        }
    }
    // Update instance and unreduction_operations.
    VertexId n = instance.number_of_vertices() - removed_vertices.size();
    reduction_output.instance = new Instance(n);
    reduction_output.unreduction_operations = std::vector<UnreductionOperations>(n);
    // Add vertices.
    std::vector<VertexId> original2reduced(instance.number_of_vertices(), -1);
    VertexId v_new = 0;
    for (auto it = removed_vertices.out_begin(); it != removed_vertices.out_end(); ++it) {
        VertexId v = *it;
        original2reduced[v] = v_new;
        reduction_output.instance->set_weight(v_new, instance.vertex(v).weight);
        reduction_output.unreduction_operations[v_new]
            = reduction_output_old.unreduction_operations[v];
        v_new++;
    }
    // Add edges.
    for (EdgeId e = 0; e < instance.number_of_edges(); ++e) {
        VertexId v1 = instance.edge(e).v1;
        VertexId v2 = instance.edge(e).v2;
        if (removed_vertices.contains(v1) || removed_vertices.contains(v2))
            continue;
        VertexId v1_new = original2reduced[v1];
        VertexId v2_new = original2reduced[v2];
        reduction_output.instance->add_edge(v1_new, v2_new, 0);
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
    for (VertexId v = 0; v < instance.number_of_vertices(); ++v) {
        // Minimum weight in S.
        // The unconfined reduction rule remains true while the minimum weight
        // in S is greater or equal to the maximum weight in N(S).
        Weight ws_min = instance.vertex(v).weight;
        Weight wns_max = 0;

        s.clear();
        n_s.clear();

        // Update S.
        s.add(v);
        // Update N(S).
        for (const auto& edge: instance.vertex(v).edges) {
            n_s.add(edge.v);
            if (wns_max < instance.vertex(edge.v).weight)
                wns_max = instance.vertex(edge.v).weight;
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
                    if (s.contains(edge.v))
                        n_u_inter_s_card++;
                if (n_u_inter_s_card != 1)
                    continue;

                VertexPos n_u_minus_n_s_card = 0;
                VertexPos w = -1;
                for (const auto& edge: instance.vertex(u).edges) {
                    if (!s.contains(edge.v)
                            && !n_s.contains(edge.v)) {
                        n_u_minus_n_s_card++;
                        w = edge.v;
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
                    if (!s.contains(edge.v)) {
                        n_s.add(edge.v);
                        if (wns_max < instance.vertex(edge.v).weight)
                            wns_max = instance.vertex(edge.v).weight;
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
            removed_vertices.add(v);
    }
    //std::cout << folded_vertices.number_of_elements() << std::endl;

    ReductionOutput reduction_output;
    if (removed_vertices.size() == 0)
        return reduction_output;

    // Update mandatory_vertices.
    reduction_output.mandatory_vertices = reduction_output_old.mandatory_vertices;
    for (VertexId v: removed_vertices) {
        for (VertexId v2: reduction_output_old.unreduction_operations[v].out) {
            reduction_output.mandatory_vertices.push_back(v2);
        }
    }
    // Update instance and unreduction_operations.
    VertexId n = instance.number_of_vertices() - removed_vertices.size();
    reduction_output.instance = new Instance(n);
    reduction_output.unreduction_operations = std::vector<UnreductionOperations>(n);
    // Add vertices.
    std::vector<VertexId> original2reduced(instance.number_of_vertices(), -1);
    VertexId v_new = 0;
    for (auto it = removed_vertices.out_begin(); it != removed_vertices.out_end(); ++it) {
        VertexId v = *it;
        original2reduced[v] = v_new;
        reduction_output.instance->set_weight(v_new, instance.vertex(v).weight);
        reduction_output.unreduction_operations[v_new]
            = reduction_output_old.unreduction_operations[v];
        v_new++;
    }
    // Add edges.
    for (EdgeId e = 0; e < instance.number_of_edges(); ++e) {
        VertexId v1 = instance.edge(e).v1;
        VertexId v2 = instance.edge(e).v2;
        if (removed_vertices.contains(v1) || removed_vertices.contains(v2))
            continue;
        VertexId v1_new = original2reduced[v1];
        VertexId v2_new = original2reduced[v2];
        reduction_output.instance->add_edge(v1_new, v2_new, 0);
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
    for (VertexId v = 0; v < number_of_vertices(); ++v)
        reduction_output_.unreduction_operations[v].in.push_back(v);

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
    for (VertexId v: reduction_output_.mandatory_vertices)
        extra_weight_ += vertex(v).weight;
    for (VertexId v = 0; v < reduction_output_.instance->number_of_vertices(); ++v)
        extra_weight_ += reduction_output_.unreduction_operations[v].out.size();
}

void stablesolver::init_display(
        const Instance& instance,
        optimizationtools::Info& info)
{
    VertexId n = instance.number_of_vertices();
    EdgeId m = instance.number_of_edges();
    FFOT_VER(info,
               "=====================================" << std::endl
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
            << std::endl);

    const Instance* reduced_instance = instance.reduced_instance();
    if (reduced_instance != nullptr) {
        VertexId n = reduced_instance->number_of_vertices();
        EdgeId m = reduced_instance->number_of_edges();
        FFOT_VER(info,
                   "Reduced instance" << std::endl
                << "----------------" << std::endl
                << "Number of vertices:              " << n << std::endl
                << "Number of edges:                 " << m << std::endl
                << "Density:                         " << (double)m * 2 / n / (n - 1) << std::endl
                << "Average degree:                  " << (double)reduced_instance->number_of_edges() * 2 / n << std::endl
                << "Maximum degree:                  " << reduced_instance->maximum_degree() << std::endl
                << "Extra weight:                    " << instance.extra_weight() << std::endl
                << "Number of connected components:  " << reduced_instance->number_of_components() << std::endl
                << std::endl);
    }
}

