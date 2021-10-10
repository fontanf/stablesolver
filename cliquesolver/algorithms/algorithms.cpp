#include "cliquesolver/algorithms/algorithms.hpp"

#include <boost/program_options.hpp>

using namespace cliquesolver;
namespace po = boost::program_options;

Output cliquesolver::run(
        std::string algorithm,
        const Instance& instance,
        std::mt19937_64& generator,
        optimizationtools::Info info)
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
    } else if (algorithm_args[0] == "greedy_strong") {
        return greedy_strong(instance, info);

#if CPLEX_FOUND
    } else if (algorithm_args[0] == "milp_cplex") {
        MilpCplexOptionalParameters parameters;
        parameters.info = info;
        return milp_cplex(instance, parameters);
#endif

    } else {
        std::cerr << "\033[31m" << "ERROR, unknown algorithm: " << algorithm_argv[0] << "\033[0m" << std::endl;
        assert(false);
        return Output(instance, info);
    }
}

