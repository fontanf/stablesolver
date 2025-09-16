#pragma once

#include "stablesolver/stable/algorithm.hpp"

#include "mathoptsolverscmake/milp.hpp"

namespace stablesolver
{
namespace stable
{

struct MilpParameters: Parameters
{
    mathoptsolverscmake::SolverName solver = mathoptsolverscmake::SolverName::Highs;


    virtual int format_width() const override { return 37; }

    virtual void format(std::ostream& os) const override
    {
        Parameters::format(os);
        int width = format_width();
        os
            << std::setw(width) << std::left << "Solver: " << solver << std::endl
            ;
    }

    virtual nlohmann::json to_json() const override
    {
        nlohmann::json json = Parameters::to_json();
        json.merge_patch({
                //{"Solver", solver},
                });
        return json;
    }
};

const Output milp_1(
        const Instance& instance,
        const MilpParameters& parameters = {});

const Output milp_2(
        const Instance& instance,
        const MilpParameters& parameters = {});

const Output milp_3(
        const Instance& instance,
        const MilpParameters& parameters = {});

void write_mps_1(
        const Instance& instance,
        mathoptsolverscmake::SolverName solver,
        const std::string& output_path);

void write_mps_2(
        const Instance& instance,
        mathoptsolverscmake::SolverName solver,
        const std::string& output_path);

void write_mps_3(
        const Instance& instance,
        mathoptsolverscmake::SolverName solver,
        const std::string& output_path);

}
}
