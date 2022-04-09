#include "stablesolver/algorithms/algorithms.hpp"

#include <boost/program_options.hpp>

using namespace stablesolver;
namespace po = boost::program_options;

LocalSearchOptionalParameters read_localsearch_args(const std::vector<char*>& argv)
{
    LocalSearchOptionalParameters parameters;
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

LocalSearchRowWeighting1OptionalParameters read_localsearch_rowweighting_1_args(const std::vector<char*>& argv)
{
    LocalSearchRowWeighting1OptionalParameters parameters;
    po::options_description desc("Allowed options");
    desc.add_options()
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

LocalSearchRowWeighting2OptionalParameters read_localsearch_rowweighting_2_args(const std::vector<char*>& argv)
{
    LocalSearchRowWeighting2OptionalParameters parameters;
    po::options_description desc("Allowed options");
    desc.add_options()
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

LargeNeighborhoodSearchOptionalParameters read_largeneighborhoodsearch_args(const std::vector<char*>& argv)
{
    LargeNeighborhoodSearchOptionalParameters parameters;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("iteration-limit,i", po::value<Counter>(&parameters.maximum_number_of_iterations), "")
        ("iteration-without-improvment-limit,w", po::value<Counter>(&parameters.maximum_number_of_iterations_without_improvement), "")
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

Output stablesolver::run(
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

    } else if (algorithm_args[0] == "greedy_gwmin") {
        return greedy_gwmin(instance, info);
    } else if (algorithm_args[0] == "greedy_gwmax") {
        return greedy_gwmax(instance, info);
    } else if (algorithm_args[0] == "greedy_gwmin2") {
        return greedy_gwmin2(instance, info);
    } else if (algorithm_args[0] == "greedy_strong") {
        return greedy_strong(instance, info);

#if CPLEX_FOUND
    } else if (algorithm_args[0] == "milp_1_cplex") {
        MilpCplexOptionalParameters parameters;
        parameters.info = info;
        return milp_1_cplex(instance, parameters);
    } else if (algorithm_args[0] == "milp_2_cplex") {
        MilpCplexOptionalParameters parameters;
        parameters.info = info;
        return milp_2_cplex(instance, parameters);
    } else if (algorithm_args[0] == "milp_3_cplex") {
        MilpCplexOptionalParameters parameters;
        parameters.info = info;
        return milp_3_cplex(instance, parameters);
#endif

    } else if (algorithm_args[0] == "localsearch") {
        auto parameters = read_localsearch_args(algorithm_argv);
        parameters.info = info;
        return localsearch(instance, generator, parameters);
    } else if (algorithm_args[0] == "localsearch_rowweighting_1") {
        auto parameters = read_localsearch_rowweighting_1_args(algorithm_argv);
        parameters.info = info;
        return localsearch_rowweighting_1(instance, generator, parameters);
    } else if (algorithm_args[0] == "localsearch_rowweighting_2") {
        auto parameters = read_localsearch_rowweighting_2_args(algorithm_argv);
        parameters.info = info;
        return localsearch_rowweighting_2(instance, generator, parameters);
    } else if (algorithm_args[0] == "largeneighborhoodsearch") {
        auto parameters = read_largeneighborhoodsearch_args(algorithm_argv);
        parameters.info = info;
        return largeneighborhoodsearch(instance, parameters);

    } else {
        throw std::invalid_argument(
                "Unknown algorithm \"" + algorithm_args[0] + "\".");
    }
}

