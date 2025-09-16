#include "stablesolver/stable/instance_builder.hpp"
#include "stablesolver/stable/algorithms/milp.hpp"

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#include <string>

using namespace stablesolver;
using namespace stablesolver::stable;
namespace po = boost::program_options;

int main(int argc, char *argv[])
{
    // Parse program options
    po::options_description desc("Allowed options");
    desc.add_options()
        (",h", "Produce help message")
        ("input,i", po::value<std::string>()->required(), "set input path")
        ("format,f", po::value<std::string>()->required(), "set input format")
        ("solver,", po::value<mathoptsolverscmake::SolverName>()->required(), "set solver")
        ("model,", po::value<int>()->required(), "set model")
        ("output,o", po::value<std::string>()->required(), "set output path")
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
    Instance instance = instance_builder.build();

    int model = 1;
    if (vm.count("model"))
        model = vm["model"].as<int>();
    if (model == 1) {
        write_mps_1(
                instance,
                vm["solver"].as<mathoptsolverscmake::SolverName>(),
                vm["output"].as<std::string>());
    } else if (model == 2) {
        write_mps_2(
                instance,
                vm["solver"].as<mathoptsolverscmake::SolverName>(),
                vm["output"].as<std::string>());
    } else if (model == 3) {
        write_mps_3(
                instance,
                vm["solver"].as<mathoptsolverscmake::SolverName>(),
                vm["output"].as<std::string>());
    }

    return 0;
}
