#include "stablesolver/clique/algorithms/algorithms.hpp"

#include <boost/program_options.hpp>

using namespace stablesolver::clique;
namespace po = boost::program_options;

LocalSearchParameters read_local_search_args(const std::vector<char*>& argv)
{
    LocalSearchParameters parameters;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("threads,t", po::value<Counter>(&parameters.number_of_threads), "")
        ;
    po::variables_map vm;
    po::store(po::parse_command_line((Counter)argv.size(), argv.data(), desc), vm);
    try {
        po::notify(vm);
    } catch (const po::required_option& e) {
        std::cout << desc << std::endl;;
        throw "";
    }
    return parameters;
}

Output stablesolver::clique::run(
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
        throw std::invalid_argument("Missing algorithm.");

    } else if (algorithm_args[0] == "greedy-gwmin") {
        return greedy_gwmin(instance, info);
    } else if (algorithm_args[0] == "greedy-strong") {
        return greedy_strong(instance, info);

#if CPLEX_FOUND
    } else if (algorithm_args[0] == "milp-cplex") {
        MilpCplexParameters parameters;
        parameters.info = info;
        return milp_cplex(instance, parameters);
#endif

    } else if (algorithm_args[0] == "local-search") {
        auto parameters = read_local_search_args(algorithm_argv);
        parameters.info = info;
        return local_search(instance, generator, parameters);

    } else {
        throw std::invalid_argument(
                "Unknown algorithm \"" + algorithm_args[0] + "\".");
    }
}

