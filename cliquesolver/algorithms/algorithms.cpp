#include "cliquesolver/algorithms/algorithms.hpp"

#include <boost/program_options.hpp>

using namespace cliquesolver;
namespace po = boost::program_options;

LocalSearchOptionalParameters read_localsearch_args(const std::vector<char*>& argv)
{
    LocalSearchOptionalParameters parameters;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("threads,t", po::value<Counter>(&parameters.thread_number), "")
        ("iterations,i", po::value<Counter>(&parameters.iteration_limit), "")
        ("iterations-without-improvment,w", po::value<Counter>(&parameters.iteration_without_improvment_limit), "")
        ;
    po::variables_map vm;
    po::store(po::parse_command_line((Counter)argv.size(), argv.data(), desc), vm);
    try {
        po::notify(vm);
    } catch (po::required_option e) {
        std::cout << desc << std::endl;;
        throw "";
    }
    return parameters;
}

Output cliquesolver::run(
        std::string algorithm, const Instance& instance, std::mt19937_64& generator, Info info)
{
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
    } else if (algorithm_args[0] == "branchandcut_cplex") {
        BranchAndCutCplexOptionalParameters parameters;
        parameters.info = info;
        return branchandcut_cplex(instance, parameters);
#endif
    } else if (algorithm_args[0] == "localsearch") {
        auto parameters = read_localsearch_args(algorithm_argv);
        parameters.info = info;
        return localsearch(instance, generator, parameters);

    } else {
        std::cerr << "\033[31m" << "ERROR, unknown algorithm: " << algorithm_argv[0] << "\033[0m" << std::endl;
        assert(false);
        return Output(instance, info);
    }
}

