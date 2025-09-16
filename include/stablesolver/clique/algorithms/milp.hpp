#pragma once

#include "stablesolver/clique/solution.hpp"

#include "mathoptsolverscmake/milp.hpp"

namespace stablesolver
{
namespace clique
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

const Output milp(
        const Instance& instance,
        const MilpParameters& parameters = {});

}
}
