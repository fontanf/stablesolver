#include "stablesolver/instance.hpp"

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
    } else {
        throw std::invalid_argument(
                "Unknown instance format \"" + format + "\".");
    }

    compute_components();
}

Instance::Instance(VertexId number_of_vertices):
    vertices_(number_of_vertices),
    total_weight_(number_of_vertices)
{
    for (VertexId v = 0; v < number_of_vertices; ++v)
        vertices_[v].id = v;
}

void Instance::add_edge(VertexId v1, VertexId v2)
{
    if (v1 == v2) {
        std::cerr << "\033[33m" << "WARNING, loop (" << v1 << ", " << v2 << ") ignored." << "\033[0m" << std::endl;
        return;
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
        line = optimizationtools::split(tmp, ' ');
        if (line.size() == 0 || tmp[0] == '%')
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

Instance Instance::complementary()
{
    Instance instance(number_of_vertices());
    optimizationtools::IndexedSet neighbors(number_of_vertices());

    for (VertexId v = 0; v < number_of_vertices(); ++v){
        instance.set_weight(v, vertex(v).weight);
        neighbors.clear();
        for (const auto& edge: vertex(v).edges)
            if (edge.v > v)
                neighbors.add(edge.v);
        for (auto it = neighbors.out_begin(); it != neighbors.out_end(); ++it)
            instance.add_edge(v, *it);
    }

    return instance;
}

void Instance::compute_components()
{
    for (ComponentId c = 0;; ++c) {
        VertexId v = 0;
        while (v < number_of_vertices()
                && (vertex(v).component != -1))
            v++;
        if (v == number_of_vertices())
            break;
        components_.push_back(Component());
        components_.back().id = c;
        std::vector<VertexId> stack {v};
        vertices_[v].component = c;
        while (!stack.empty()) {
            v = stack.back();
            stack.pop_back();
            for (const auto& edge: vertex(v).edges) {
                edges_[edge.e].component = c;
                if (vertex(edge.v).component != -1)
                    continue;
                vertices_[edge.v].component = c;
                stack.push_back(edge.v);
            }
        }
    }

    for (VertexId v = 0; v < number_of_vertices(); ++v)
        if (vertex(v).component != -1)
            components_[vertex(v).component].vertices.push_back(v);
    for (EdgeId e = 0; e < number_of_edges(); ++e)
        components_[edge(e).component].edges.push_back(e);
}

