#include "stablesolver/stable/algorithms/algorithms.hpp"

#include <boost/program_options.hpp>

using namespace stablesolver::stable;
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

LocalSearchRowWeighting1Parameters read_local_search_row_weighting_1_args(const std::vector<char*>& argv)
{
    LocalSearchRowWeighting1Parameters parameters;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("iterations,i", po::value<Counter>(&parameters.maximum_number_of_iterations), "")
        ("iterations-without-improvement,w", po::value<Counter>(&parameters.maximum_number_of_iterations_without_improvement), "")
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

LocalSearchRowWeighting2Parameters read_local_search_row_weighting_2_args(const std::vector<char*>& argv)
{
    LocalSearchRowWeighting2Parameters parameters;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("iterations,i", po::value<Counter>(&parameters.maximum_number_of_iterations), "")
        ("iterations-without-improvement,w", po::value<Counter>(&parameters.maximum_number_of_iterations_without_improvement), "")
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

LargeNeighborhoodSearchParameters read_large_neighborhood_search_args(const std::vector<char*>& argv)
{
    LargeNeighborhoodSearchParameters parameters;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("iterations,i", po::value<Counter>(&parameters.maximum_number_of_iterations), "")
        ("iterations-without-improvement,w", po::value<Counter>(&parameters.maximum_number_of_iterations_without_improvement), "")
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

Output stablesolver::stable::run(
        std::string algorithm,
        Instance& instance,
        std::mt19937_64& generator,
        optimizationtools::Info info)
{
    std::vector<std::string> algorithm_args = po::split_unix(algorithm);
    std::vector<char*> algorithm_argv;
    for (Counter i = 0; i < (Counter)algorithm_args.size(); ++i)
        algorithm_argv.push_back(const_cast<char*>(algorithm_args[i].c_str()));

    if (algorithm.empty() || algorithm_args[0].empty()) {
        throw std::invalid_argument("Missing algorithm.");

    } else if (algorithm_args[0] == "greedy-gwmin") {
        GreedyParameters parameters;
        parameters.info = info;
        return greedy_gwmin(instance, parameters);
    } else if (algorithm_args[0] == "greedy-gwmax") {
        GreedyParameters parameters;
        parameters.info = info;
        return greedy_gwmax(instance, parameters);
    } else if (algorithm_args[0] == "greedy-gwmin2") {
        GreedyParameters parameters;
        parameters.info = info;
        return greedy_gwmin2(instance, parameters);
    } else if (algorithm_args[0] == "greedy-strong") {
        GreedyParameters parameters;
        parameters.info = info;
        return greedy_strong(instance, parameters);

#if COINOR_FOUND
    } else if (algorithm_args[0] == "milp-1-cbc") {
        MilpCbcParameters parameters;
        parameters.info = info;
        return milp_1_cbc(instance, parameters);
#endif
#if CPLEX_FOUND
    } else if (algorithm_args[0] == "milp-1-cplex") {
        MilpCplexParameters parameters;
        parameters.info = info;
        return milp_1_cplex(instance, parameters);
    } else if (algorithm_args[0] == "milp-2-cplex") {
        MilpCplexParameters parameters;
        parameters.info = info;
        return milp_2_cplex(instance, parameters);
    } else if (algorithm_args[0] == "milp-3-cplex") {
        MilpCplexParameters parameters;
        parameters.info = info;
        return milp_3_cplex(instance, parameters);
#endif

    } else if (algorithm_args[0] == "local-search") {
        auto parameters = read_local_search_args(algorithm_argv);
        parameters.info = info;
        return local_search(instance, generator, parameters);
    } else if (algorithm_args[0] == "local-search-row-weighting-1") {
        auto parameters = read_local_search_row_weighting_1_args(algorithm_argv);
        parameters.info = info;
        return local_search_row_weighting_1(instance, generator, parameters);
    } else if (algorithm_args[0] == "local-search-row-weighting-2") {
        auto parameters = read_local_search_row_weighting_2_args(algorithm_argv);
        parameters.info = info;
        return local_search_row_weighting_2(instance, generator, parameters);
    } else if (algorithm_args[0] == "large-neighborhood-search") {
        auto parameters = read_large_neighborhood_search_args(algorithm_argv);
        parameters.info = info;
        return large_neighborhood_search(instance, parameters);

    } else {
        throw std::invalid_argument(
                "Unknown algorithm \"" + algorithm_args[0] + "\".");
    }
}

