#include "stablesolver/instance.hpp"

#include <sstream>
#include <iomanip>
#include <thread>

using namespace stablesolver;

Instance::Instance(std::string filepath, std::string format)
{
    std::ifstream file(filepath);
    if (!file.good()) {
        std::cerr << "\033[31m" << "ERROR, unable to open file \"" << filepath << "\"" << "\033[0m" << std::endl;
        assert(false);
        return;
    }

    if (format == "dimacs2010") {
        read_dimacs2010(file);
    } else {
        std::cerr << "\033[31m" << "ERROR, unknown instance format: \"" << format << "\"" << "\033[0m" << std::endl;
        assert(false);
    }

    compute_components();
}

Instance::Instance(VertexId vertex_number):
    vertices_(vertex_number),
    weight_total_(vertex_number)
{
    for (VertexId v = 0; v < vertex_number; ++v)
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
    if (degree_max_ < (VertexId)vertices_[v1].edges.size())
        degree_max_ = vertices_[v1].edges.size();

    VertexEdge ve2;
    ve2.e = e.id;
    ve2.v = v1;
    vertices_[v2].edges.push_back(ve2);
    if (degree_max_ < (VertexId)vertices_[v2].edges.size())
        degree_max_ = vertices_[v2].edges.size();
}

void Instance::set_weight(VertexId v, Weight w)
{
    weight_total_ -= vertex(v).weight;
    vertices_[v].weight = w;
    weight_total_ += vertex(v).weight;
}

void Instance::set_unweighted()
{
    for (VertexId v = 0; v < vertex_number(); ++v)
        vertices_[v].weight = 1;
    weight_total_ = vertex_number();
}

void Instance::read_dimacs2010(std::ifstream& file)
{
    std::string tmp;
    std::vector<std::string> line;
    bool first = true;
    VertexId v = -1;
    while (v != vertex_number()) {
        getline(file, tmp);
        line = optimizationtools::split(tmp, ' ');
        if (line.size() == 0 || tmp[0] == '%')
            continue;
        if (first) {
            VertexId vertex_number = stol(line[0]);
            vertices_.resize(vertex_number);
            for (VertexId v = 0; v < vertex_number; ++v)
                vertices_[v].id = v;
            weight_total_ = vertex_number;
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

Instance Instance::complementary()
{
    Instance instance(vertex_number());
    optimizationtools::IndexedSet neighbors(vertex_number());

    for (VertexId v = 0; v < vertex_number(); ++v){
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
    std::cout << "compute_components" << std::endl;
    for (ComponentId c = 0;; ++c) {
        VertexId v = 0;
        while (v < vertex_number()
                && (vertex(v).component != -1))
            v++;
        if (v == vertex_number())
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

    for (VertexId v = 0; v < vertex_number(); ++v)
        if (vertex(v).component != -1)
            components_[vertex(v).component].vertices.push_back(v);
    for (EdgeId e = 0; e < edge_number(); ++e)
        components_[edge(e).component].edges.push_back(e);
}

