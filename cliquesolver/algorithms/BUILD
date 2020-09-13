load("//stablesolver:variables.bzl", "STDCPP",
         "CPLEX_COPTS", "CPLEX_DEP", "CPOPTIMIZER_DEP")

cc_library(
        name = "algorithms",
        hdrs = [
                "greedy.hpp",
                "algorithms.hpp",
        ],
        srcs = [
                "greedy.cpp",
                "algorithms.cpp",
        ],
        deps = [
                "//cliquesolver:cliquesolver",
        ] + CPLEX_DEP,
        copts = CPLEX_COPTS,
        visibility = ["//visibility:public"],
)

