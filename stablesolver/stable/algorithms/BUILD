load("//stablesolver:variables.bzl", "STDCPP",
         "CBC_COPTS", "CBC_DEP",
         "CPLEX_COPTS", "CPLEX_DEP")

cc_library(
        name = "greedy",
        hdrs = ["greedy.hpp"],
        srcs = ["greedy.cpp"],
        deps = ["//stablesolver/stable:stable"],
        visibility = ["//visibility:public"],
)

cc_library(
        name = "milp_cbc",
        hdrs = ["milp_cbc.hpp"],
        srcs = ["milp_cbc.cpp"],
        deps = [
                "//stablesolver/stable:stable",
        ] + CBC_DEP,
        copts = CBC_COPTS,
        visibility = ["//visibility:public"],
)

cc_library(
        name = "milp_cplex",
        hdrs = ["milp_cplex.hpp"],
        srcs = ["milp_cplex.cpp"],
        deps = [
                "//stablesolver/stable:stable",
                "//stablesolver/clique/algorithms:algorithms",
        ] + CPLEX_DEP,
        copts = CPLEX_COPTS,
        visibility = ["//visibility:public"],
)

cc_library(
        name = "local_search",
        hdrs = ["local_search.hpp"],
        srcs = ["local_search.cpp"],
        deps = [
                "//stablesolver/stable:stable",
                "@localsearchsolver//localsearchsolver:localsearchsolver",
        ],
        visibility = ["//visibility:public"],
)

cc_library(
        name = "local_search_row_weighting",
        hdrs = ["local_search_row_weighting.hpp"],
        srcs = ["local_search_row_weighting.cpp"],
        deps = [
                "//stablesolver/stable:stable",
                ":greedy",
        ],
        visibility = ["//visibility:public"],
)

cc_library(
        name = "large_neighborhood_search",
        hdrs = ["large_neighborhood_search.hpp"],
        srcs = ["large_neighborhood_search.cpp"],
        deps = [
                "//stablesolver/stable:stable",
                ":greedy",
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
        ] + CBC_DEP + CPLEX_DEP,
        copts = CBC_COPTS + CPLEX_COPTS,
        visibility = ["//visibility:public"],
)

