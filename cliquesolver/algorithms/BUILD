load("//stablesolver:variables.bzl", "STDCPP",
         "CPLEX_COPTS", "CPLEX_DEP", "CPOPTIMIZER_DEP")

cc_library(
        name = "algorithms",
        hdrs = [
                "greedy.hpp",
                "localsearch.hpp",
                "branchandcut_cplex.hpp",
                "algorithms.hpp",
        ],
        srcs = [
                "greedy.cpp",
                "localsearch.cpp",
                "branchandcut_cplex.cpp",
                "algorithms.cpp",
        ],
        deps = [
                "//cliquesolver:cliquesolver",
        ] + CPLEX_DEP,
        copts = CPLEX_COPTS,
        visibility = ["//visibility:public"],
)

