#include "stablesolver/stable/instance_builder.hpp"

#include "stablesolver/stable/solution.hpp"
#include "stablesolver/stable/algorithms/greedy.hpp"
#if CBC_FOUND
#include "stablesolver/stable/algorithms/milp_cbc.hpp"
#endif
#if CPLEX_FOUND
#include "stablesolver/stable/algorithms/milp_cplex.hpp"
#endif
#include "stablesolver/stable/algorithms/local_search.hpp"
#include "stablesolver/stable/algorithms/local_search_row_weighting.hpp"
#include "stablesolver/stable/algorithms/large_neighborhood_search.hpp"

#include <boost/program_options.hpp>

using namespace stablesolver::stable;

namespace po = boost::program_options;

void read_args(
        Parameters& parameters,
        const po::variables_map& vm)
{
    parameters.timer.set_sigint_handler();
    parameters.messages_to_stdout = true;
    if (vm.count("time-limit"))
        parameters.timer.set_time_limit(vm["time-limit"].as<double>());
    if (vm.count("verbosity-level"))
        parameters.verbosity_level = vm["verbosity-level"].as<int>();
    if (vm.count("log"))
        parameters.log_path = vm["log"].as<std::string>();
    parameters.log_to_stderr = vm.count("log-to-stderr");
    bool only_write_at_the_end = vm.count("only-write-at-the-end");
    if (!only_write_at_the_end) {
        std::string certificate_path = vm["certificate"].as<std::string>();
        std::string json_output_path = vm["output"].as<std::string>();
        parameters.new_solution_callback = [
            json_output_path,
            certificate_path](
                    const Output& output,
                    const std::string&)
        {
            output.write_json_output(json_output_path);
            output.solution.write(certificate_path);
        };
    }
}

Output run(
        const Instance& instance,
        const po::variables_map& vm)
{
    std::mt19937_64 generator(vm["seed"].as<Seed>());
    Solution solution(instance, vm["initial-solution"].as<std::string>());

    // Run algorithm.
    std::string algorithm = vm["algorithm"].as<std::string>();
    if (algorithm == "greedy-gwmin") {
        GreedyParameters parameters;
        read_args(parameters, vm);
        return greedy_gwmin(instance, parameters);
    } else if (algorithm == "greedy-gwmax") {
        GreedyParameters parameters;
        read_args(parameters, vm);
        return greedy_gwmax(instance, parameters);
    } else if (algorithm == "greedy-gwmin2") {
        GreedyParameters parameters;
        read_args(parameters, vm);
        return greedy_gwmin2(instance, parameters);
    } else if (algorithm == "greedy-strong") {
        GreedyParameters parameters;
        read_args(parameters, vm);
        return greedy_strong(instance, parameters);
#if CBC_FOUND
    } else if (algorithm == "milp-cbc") {
        MilpCbcParameters parameters;
        read_args(parameters, vm);
        return milp_1_cbc(instance, parameters);
#endif
#if CPLEX_FOUND
    } else if (algorithm == "milp-1-cplex") {
        MilpCplexParameters parameters;
        read_args(parameters, vm);
        return milp_1_cplex(instance, parameters);
    } else if (algorithm == "milp-2-cplex") {
        MilpCplexParameters parameters;
        read_args(parameters, vm);
        return milp_1_cplex(instance, parameters);
    } else if (algorithm == "milp-3-cplex") {
        MilpCplexParameters parameters;
        read_args(parameters, vm);
        return milp_1_cplex(instance, parameters);
#endif
    } else if (algorithm == "local-search-row-weighting-1") {
        LocalSearchRowWeighting1Parameters parameters;
        read_args(parameters, vm);
        if (vm.count("maximum-number-of-iterations"))
            parameters.maximum_number_of_iterations = vm["maximum-number-of-iterations"].as<int>();
        if (vm.count("maximum-number-of-iterations-without-improvement"))
            parameters.maximum_number_of_iterations_without_improvement = vm["maximum-number-of-iterations-without-improvement"].as<int>();
        return local_search_row_weighting_1(instance, generator, parameters);
    } else if (algorithm == "local-search-row-weighting-2") {
        LocalSearchRowWeighting2Parameters parameters;
        read_args(parameters, vm);
        if (vm.count("maximum-number-of-iterations"))
            parameters.maximum_number_of_iterations = vm["maximum-number-of-iterations"].as<int>();
        if (vm.count("maximum-number-of-iterations-without-improvement"))
            parameters.maximum_number_of_iterations_without_improvement = vm["maximum-number-of-iterations-without-improvement"].as<int>();
        return local_search_row_weighting_2(instance, generator, parameters);
    } else if (algorithm == "local-search") {
        LocalSearchParameters parameters;
        read_args(parameters, vm);
        return local_search(instance, parameters);
    } else if (algorithm == "large-neighborhood-search") {
        LargeNeighborhoodSearchParameters parameters;
        read_args(parameters, vm);
        if (vm.count("maximum-number-of-iterations"))
            parameters.maximum_number_of_iterations = vm["maximum-number-of-iterations"].as<int>();
        if (vm.count("maximum-number-of-iterations-without-improvement"))
            parameters.maximum_number_of_iterations_without_improvement = vm["maximum-number-of-iterations-without-improvement"].as<int>();
        return large_neighborhood_search(instance, parameters);

    } else {
        throw std::invalid_argument(
                "Unknown algorithm \"" + algorithm + "\".");
    }
}

int main(int argc, char *argv[])
{
    // Parse program options
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "produce help message")
        ("algorithm,a", po::value<std::string>()->default_value("large-neighborhood-search"), "set algorithm")
        ("input,i", po::value<std::string>()->required(), "set input file (required)")
        ("format,f", po::value<std::string>()->default_value(""), "set input file format (default: standard)")
        ("unweighted,u", "set unweighted")
        ("complementary", "set complementary")
        ("output,o", po::value<std::string>()->default_value(""), "set JSON output file")
        ("initial-solution,", po::value<std::string>()->default_value(""), "")
        ("certificate,c", po::value<std::string>()->default_value(""), "set certificate file")
        ("seed,s", po::value<Seed>()->default_value(0), "set seed")
        ("time-limit,t", po::value<double>(), "set time limit in seconds")
        ("verbosity-level,v", po::value<int>(), "set verbosity level")
        ("only-write-at-the-end,e", "only write output and certificate files at the end")
        ("log,l", po::value<std::string>(), "set log file")
        ("log-to-stderr", "write log to stderr")

        ("maximum-number-of-iterations,", po::value<int>(), "set the maximum number of iterations")
        ("maximum-number-of-iterations-without-improvement,", po::value<int>(), "set the maximum number of iterations without improvement")
        ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    if (vm.count("help")) {
        std::cout << desc << std::endl;;
        return 1;
    }
    try {
        po::notify(vm);
    } catch (const po::required_option& e) {
        std::cout << desc << std::endl;;
        return 1;
    }

    // Build instance.
    InstanceBuilder instance_builder;
    instance_builder.read(
            vm["input"].as<std::string>(),
            vm["format"].as<std::string>());
    if (vm.count("unweighted"))
        instance_builder.set_unweighted();
    const Instance instance = (!vm.count("complementary"))?
        instance_builder.build():
        instance_builder.build().complementary();

    // Run.
    Output output = run(instance, vm);

    // Write outputs.
    std::string certificate_path = vm["certificate"].as<std::string>();
    std::string json_output_path = vm["output"].as<std::string>();
    output.write_json_output(json_output_path);
    output.solution.write(certificate_path);

    return 0;
}
