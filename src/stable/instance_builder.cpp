#include "stablesolver/stable/instance_builder.hpp"

#include "optimizationtools/utils/utils.hpp"

#include <fstream>

using namespace stablesolver::stable;

void InstanceBuilder::add_vertices(VertexId number_of_vertices)
{
    instance_.vertices_.insert(instance_.vertices_.end(), number_of_vertices, Vertex());
}

void InstanceBuilder::add_vertex(Weight weight)
{
    Vertex vertex;
    vertex.weight = weight;
    instance_.vertices_.push_back(vertex);
}

void InstanceBuilder::add_edge(
        VertexId vertex_id_1,
        VertexId vertex_id_2,
        int check_duplicate)
{
    if (check_duplicate > 0) {
        for (const auto& edge: instance_.vertex(vertex_id_1).edges) {
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

    EdgeId edge_id = instance_.edges_.size();

    Edge e;
    e.vertex_id_1 = vertex_id_1;
    e.vertex_id_2 = vertex_id_2;
    instance_.edges_.push_back(e);

    VertexEdge ve1;
    ve1.edge_id = edge_id;
    ve1.vertex_id = vertex_id_2;
    instance_.vertices_[vertex_id_1].edges.push_back(ve1);

    VertexEdge ve2;
    ve2.edge_id = edge_id;
    ve2.vertex_id = vertex_id_1;
    instance_.vertices_[vertex_id_2].edges.push_back(ve2);
}

void InstanceBuilder::set_weight(
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

    instance_.vertices_[vertex_id].weight = weight;
}

void InstanceBuilder::set_unweighted()
{
    for (VertexId vertex_id = 0;
            vertex_id < instance_.number_of_vertices();
            ++vertex_id)
        instance_.vertices_[vertex_id].weight = 1;
}

void InstanceBuilder::read(
        std::string instance_path,
        std::string format)
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

void InstanceBuilder::read_dimacs1992(std::ifstream& file)
{
    std::string tmp;
    std::vector<std::string> line;

    while (getline(file, tmp)) {
        line = optimizationtools::split(tmp, ' ');
        if (line.size() == 0) {
        } else if (line[0] == "c") {
            if (instance_.name_ == "")
                instance_.name_ = line.back();
        } else if (line[0] == "p") {
            VertexId number_of_vertices = stol(line[2]);
            for (VertexId vertex_id = 0;
                    vertex_id < number_of_vertices;
                    ++vertex_id) {
                add_vertex();
            }
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

void InstanceBuilder::read_dimacs2010(std::ifstream& file)
{
    std::string tmp;
    std::vector<std::string> line;
    bool first = true;
    VertexId vertex_id = -1;
    while (vertex_id != instance_.number_of_vertices()) {
        getline(file, tmp);
        //std::cout << tmp << std::endl;
        line = optimizationtools::split(tmp, ' ');
        if (tmp[0] == '%')
            continue;
        if (first) {
            VertexId number_of_vertices = stol(line[0]);
            for (VertexId vertex_id = 0;
                    vertex_id < number_of_vertices;
                    ++vertex_id) {
                add_vertex();
            }
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

void InstanceBuilder::read_matrixmarket(std::ifstream& file)
{
    std::string tmp;
    std::vector<std::string> line;
    do {
        getline(file, tmp);
    } while (tmp[0] == '%');
    line = optimizationtools::split(tmp, ' ');
    VertexId number_of_vertices = stol(line[0]);
    for (VertexId vertex_id = 0;
            vertex_id < number_of_vertices;
            ++vertex_id) {
        add_vertex();
    }

    while (getline(file, tmp)) {
        line = optimizationtools::split(tmp, ' ');
        VertexId vertex_id_1 = stol(line[0]) - 1;
        VertexId vertex_id_2 = stol(line[1]) - 1;
        add_edge(vertex_id_1, vertex_id_2);
    }
}

void InstanceBuilder::read_chaco(std::ifstream& file)
{
    std::string tmp;
    std::vector<std::string> line;

    getline(file, tmp);
    line = optimizationtools::split(tmp, ' ');
    VertexId number_of_vertices = stol(line[0]);
    for (VertexId vertex_id = 0;
            vertex_id < number_of_vertices;
            ++vertex_id) {
        add_vertex();
    }

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

void InstanceBuilder::read_snap(std::ifstream& file)
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
        while (std::max(vertex_id_1, vertex_id_2) >= instance_.number_of_vertices())
            add_vertex();
        add_edge(vertex_id_1, vertex_id_2);
    }
}

////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// Build /////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void InstanceBuilder::compute_highest_degree()
{
    instance_.highest_degree_ = 0;
    for (VertexId vertex_id = 0;
            vertex_id < instance_.number_of_vertices();
            ++vertex_id) {
        instance_.highest_degree_ = std::max(
                instance_.highest_degree_,
                (VertexId)instance_.vertex(vertex_id).edges.size());
    }
}

void InstanceBuilder::compute_total_weight()
{
    instance_.total_weight_ = 0;
    for (VertexId vertex_id = 0;
            vertex_id < instance_.number_of_vertices();
            ++vertex_id) {
        instance_.total_weight_ += instance_.vertex(vertex_id).weight;
    }
}

void InstanceBuilder::compute_components()
{
    //std::cout << "compute_components" << std::endl;
    std::vector<VertexId> stack;
    VertexId vertex_id_0 = 0;
    for (ComponentId c = 0;; ++c) {
        while (vertex_id_0 < instance_.number_of_vertices()
                && (instance_.vertex(vertex_id_0).component != -1))
            vertex_id_0++;
        if (vertex_id_0 == instance_.number_of_vertices())
            break;
        //std::cout << "c " << c << " v " << v << std::endl;
        Component component;
        component.id = c;
        stack.clear();
        stack.push_back(vertex_id_0);
        instance_.vertices_[vertex_id_0].component = c;
        while (!stack.empty()) {
            VertexId vertex_id = stack.back();
            stack.pop_back();
            for (const auto& edge: instance_.vertex(vertex_id).edges) {
                instance_.edges_[edge.edge_id].component = c;
                if (instance_.vertex(edge.vertex_id).component != -1)
                    continue;
                instance_.vertices_[edge.vertex_id].component = c;
                stack.push_back(edge.vertex_id);
            }
        }
        instance_.components_.push_back(component);
    }

    for (VertexId vertex_id = 0;
            vertex_id < instance_.number_of_vertices();
            ++vertex_id) {
        if (instance_.vertex(vertex_id).component != -1)
            instance_.components_[instance_.vertex(vertex_id).component].vertices.push_back(vertex_id);
    }
    for (EdgeId edge_id = 0; edge_id < instance_.number_of_edges(); ++edge_id)
        instance_.components_[instance_.edge(edge_id).component].edges.push_back(edge_id);
}

Instance InstanceBuilder::build()
{
    compute_highest_degree();
    compute_total_weight();
    compute_components();
    return std::move(instance_);
}
