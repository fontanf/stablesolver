#pragma once

#include "stablesolver/solution.hpp"

namespace stablesolver
{

/************************* decisiondiagram_restricted *************************/

struct DecisionDiagramRestrictedOptionalParameters
{
    Info info = Info();
};

struct DecisionDiagramRestrictedOutput: Output
{
    DecisionDiagramRestrictedOutput(const Instance& instance, Info& info): Output(instance, info) { }
    DecisionDiagramRestrictedOutput& algorithm_end(Info& info);
};

DecisionDiagramRestrictedOutput decisiondiagram_restricted(
        const Instance& instance,
        DecisionDiagramRestrictedOptionalParameters parameters = {});

/************************** decisiondiagram_relaxed ***************************/

struct DecisionDiagramRelaxedOptionalParameters
{
    Info info = Info();
};

struct DecisionDiagramRelaxedOutput: Output
{
    DecisionDiagramRelaxedOutput(const Instance& instance, Info& info): Output(instance, info) { }
    DecisionDiagramRelaxedOutput& algorithm_end(Info& info);
};

DecisionDiagramRelaxedOutput decisiondiagram_relaxed(
        const Instance& instance,
        DecisionDiagramRelaxedOptionalParameters parameters = {});

/************************* decisiondiagram_separation *************************/

struct DecisionDiagramSeparationOptionalParameters
{
    Info info = Info();
};

struct DecisionDiagramSeparationOutput: Output
{
    DecisionDiagramSeparationOutput(const Instance& instance, Info& info): Output(instance, info) { }
    DecisionDiagramSeparationOutput& algorithm_end(Info& info);
};

DecisionDiagramSeparationOutput decisiondiagram_separation(
        const Instance& instance,
        DecisionDiagramSeparationOptionalParameters parameters = {});

/*********************** branchandbound_decisiondiagram ***********************/

struct BranchAndBoundDecisionDiagramOptionalParameters
{
    Info info = Info();
};

struct BranchAndBoundDecisionDiagramOutput: Output
{
    BranchAndBoundDecisionDiagramOutput(const Instance& instance, Info& info): Output(instance, info) { }
    BranchAndBoundDecisionDiagramOutput& algorithm_end(Info& info);
};

BranchAndBoundDecisionDiagramOutput branchandbound_decisiondiagram(
        const Instance& instance,
        BranchAndBoundDecisionDiagramOptionalParameters parameters = {});

}

