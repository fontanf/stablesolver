load("//stablesolver:variables.bzl", "STDCPP",
         "COINOR_COPTS", "COINOR_DEP",
         "CPLEX_COPTS", "CPLEX_DEP", "CPOPTIMIZER_DEP")

cc_library(
        name = "greedy",
        hdrs = ["greedy.hpp"],
        srcs = ["greedy.cpp"],
        deps = ["//stablesolver:stablesolver"],
        visibility = ["//visibility:public"],
)

cc_library(
        name = "milp_cbc",
        hdrs = ["milp_cbc.hpp"],
        srcs = ["milp_cbc.cpp"],
        deps = [
                "//stablesolver:stablesolver",
        ] + COINOR_DEP,
        copts = COINOR_COPTS,
        visibility = ["//visibility:public"],
)

cc_library(
        name = "milp_cplex",
        hdrs = ["milp_cplex.hpp"],
        srcs = ["milp_cplex.cpp"],
        deps = [
                "//stablesolver:stablesolver",
                "//cliquesolver/algorithms:algorithms",
        ] + CPLEX_DEP,
        copts = CPLEX_COPTS,
        visibility = ["//visibility:public"],
)

cc_library(
        name = "local_search",
        hdrs = ["local_search.hpp"],
        srcs = ["local_search.cpp"],
        deps = [
                "//stablesolver:stablesolver",
                "@localsearchsolver//localsearchsolver:localsearchsolver",
        ],
        visibility = ["//visibility:public"],
)

cc_library(
        name = "local_search_row_weighting",
        hdrs = ["local_search_row_weighting.hpp"],
        srcs = ["local_search_row_weighting.cpp"],
        deps = [
                ":greedy",
                "//stablesolver:stablesolver",
        ],
        visibility = ["//visibility:public"],
)

cc_library(
        name = "large_neighborhood_search",
        hdrs = ["large_neighborhood_search.hpp"],
        srcs = ["large_neighborhood_search.cpp"],
        deps = [
                ":greedy",
                "//stablesolver:stablesolver",
        ],
        visibility = ["//visibility:public"],
)

cc_library(
        name = "algorithms",
        hdrs = ["algorithms.hpp"],
        srcs = ["algorithms.cpp"],
        deps = [
                ":greedy",
                ":milp_cbc",
                ":milp_cplex",
                ":local_search",
                ":local_search_row_weighting",
                ":large_neighborhood_search",
                "@boost//:program_options",
        ] + COINOR_DEP + CPLEX_DEP,
        copts = COINOR_COPTS + CPLEX_COPTS,
        visibility = ["//visibility:public"],
)

