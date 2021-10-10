load("//stablesolver:variables.bzl", "STDCPP",
         "CPLEX_COPTS", "CPLEX_DEP", "CPOPTIMIZER_DEP")

cc_library(
        name = "greedy",
        hdrs = ["greedy.hpp"],
        srcs = ["greedy.cpp"],
        deps = ["//cliquesolver:cliquesolver"],
        visibility = ["//visibility:public"],
)

cc_library(
        name = "milp_cplex",
        hdrs = ["milp_cplex.hpp"],
        srcs = ["milp_cplex.cpp"],
        deps = [
                "//cliquesolver:cliquesolver",
        ] + CPLEX_DEP,
        copts = CPLEX_COPTS,
        visibility = ["//visibility:public"],
)

cc_library(
        name = "algorithms",
        hdrs = ["algorithms.hpp"],
        srcs = ["algorithms.cpp"],
        deps = [
                ":greedy",
                ":milp_cplex",
                "@boost//:program_options",
        ] + CPLEX_DEP,
        copts = CPLEX_COPTS,
        visibility = ["//visibility:public"],
)

