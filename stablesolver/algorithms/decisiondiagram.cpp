#include "stablesolver/algorithms/decisiondiagram.hpp"

using namespace stablesolver;

/************************* decisiondiagram_restricted *************************/

DecisionDiagramRestrictedOutput& DecisionDiagramRestrictedOutput::algorithm_end(Info& info)
{
    //PUT(info, "Algorithm", "Iterations", iterations);
    Output::algorithm_end(info);
    //VER(info, "Iterations: " << iterations << std::endl);
    return *this;
}

DecisionDiagramRestrictedOutput stablesolver::decisiondiagram_restricted(
        const Instance& instance,
        DecisionDiagramRestrictedOptionalParameters parameters)
{
    VER(parameters.info, "*** decisiondiagram_restricted ***" << std::endl);
    DecisionDiagramRestrictedOutput output(instance, parameters.info);

    return output.algorithm_end(parameters.info);
}

/************************** decisiondiagram_relaxed ***************************/

DecisionDiagramRelaxedOutput& DecisionDiagramRelaxedOutput::algorithm_end(Info& info)
{
    //PUT(info, "Algorithm", "Iterations", iterations);
    Output::algorithm_end(info);
    //VER(info, "Iterations: " << iterations << std::endl);
    return *this;
}

DecisionDiagramRelaxedOutput stablesolver::decisiondiagram_relaxed(
        const Instance& instance,
        DecisionDiagramRelaxedOptionalParameters parameters)
{
    VER(parameters.info, "*** decisiondiagram_relaxed ***" << std::endl);
    DecisionDiagramRelaxedOutput output(instance, parameters.info);

    return output.algorithm_end(parameters.info);
}

/************************* decisiondiagram_separation *************************/

DecisionDiagramSeparationOutput& DecisionDiagramSeparationOutput::algorithm_end(Info& info)
{
    //PUT(info, "Algorithm", "Iterations", iterations);
    Output::algorithm_end(info);
    //VER(info, "Iterations: " << iterations << std::endl);
    return *this;
}

DecisionDiagramSeparationOutput stablesolver::decisiondiagram_separation(
        const Instance& instance,
        DecisionDiagramSeparationOptionalParameters parameters)
{
    VER(parameters.info, "*** decisiondiagram_separation ***" << std::endl);
    DecisionDiagramSeparationOutput output(instance, parameters.info);

    return output.algorithm_end(parameters.info);
}

/*********************** branchandbound_decisiondiagram ***********************/

BranchAndBoundDecisionDiagramOutput& BranchAndBoundDecisionDiagramOutput::algorithm_end(Info& info)
{
    //PUT(info, "Algorithm", "Iterations", iterations);
    Output::algorithm_end(info);
    //VER(info, "Iterations: " << iterations << std::endl);
    return *this;
}

BranchAndBoundDecisionDiagramOutput stablesolver::branchandbound_decisiondiagram(
        const Instance& instance,
        BranchAndBoundDecisionDiagramOptionalParameters parameters)
{
    VER(parameters.info, "*** branchandbound_decisiondiagram ***" << std::endl);
    BranchAndBoundDecisionDiagramOutput output(instance, parameters.info);

    return output.algorithm_end(parameters.info);
}

