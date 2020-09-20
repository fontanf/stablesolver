#include "stablesolver/algorithms/algorithms.hpp"

#include <boost/program_options.hpp>

using namespace stablesolver;
namespace po = boost::program_options;

Output stablesolver::run(
        std::string algorithm,
        const Instance& instance,
        std::mt19937_64& generator,
        Info info)
{
    (void)generator;

    std::vector<std::string> algorithm_args = po::split_unix(algorithm);
    std::vector<char*> algorithm_argv;
    for (Counter i = 0; i < (Counter)algorithm_args.size(); ++i)
        algorithm_argv.push_back(const_cast<char*>(algorithm_args[i].c_str()));

    if (algorithm.empty() || algorithm_args[0].empty()) {
        std::cerr << "\033[32m" << "ERROR, missing algorithm." << "\033[0m" << std::endl;
        return Output(instance, info);

    } else if (algorithm_args[0] == "greedy_gwmin") {
        return greedy_gwmin(instance, info);
    } else if (algorithm_args[0] == "greedy_gwmax") {
        return greedy_gwmax(instance, info);
    } else if (algorithm_args[0] == "greedy_gwmin2") {
        return greedy_gwmin2(instance, info);
#if CPLEX_FOUND
    } else if (algorithm_args[0] == "branchandcut_1_cplex") {
        BranchAndCutCplexOptionalParameters parameters;
        parameters.info = info;
        return branchandcut_1_cplex(instance, parameters);
    } else if (algorithm_args[0] == "branchandcut_2_cplex") {
        BranchAndCutCplexOptionalParameters parameters;
        parameters.info = info;
        return branchandcut_2_cplex(instance, parameters);
    } else if (algorithm_args[0] == "branchandcut_3_cplex") {
        BranchAndCutCplexOptionalParameters parameters;
        parameters.info = info;
        return branchandcut_3_cplex(instance, parameters);
#endif

    } else {
        std::cerr << "\033[31m" << "ERROR, unknown algorithm: " << algorithm_argv[0] << "\033[0m" << std::endl;
        assert(false);
        return Output(instance, info);
    }
}

