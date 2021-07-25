load("//stablesolver:variables.bzl", "STDCPP",
         "CPLEX_COPTS", "CPLEX_DEP", "CPOPTIMIZER_DEP")

cc_library(
        name = "algorithms",
        hdrs = [
                "greedy.hpp",
                "milp_cplex.hpp",
                "localsearch_rowweighting.hpp",
                "largeneighborhoodsearch.hpp",
                "algorithms.hpp",
        ],
        srcs = [
                "greedy.cpp",
                "milp_cplex.cpp",
                "localsearch_rowweighting.cpp",
                "largeneighborhoodsearch.cpp",
                "algorithms.cpp",
        ],
        deps = [
                "//stablesolver:stablesolver",
                "//cliquesolver/algorithms:algorithms",
                "@boost//:program_options",
        ] + CPLEX_DEP,
        copts = CPLEX_COPTS,
        visibility = ["//visibility:public"],
)

