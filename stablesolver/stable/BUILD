load("//stablesolver:variables.bzl", "STDCPP")

cc_library(
        name = "stable",
        hdrs = [
                "instance.hpp",
                "instance_builder.hpp",
                "solution.hpp",
                "reduction.hpp",
                "algorithm.hpp",
                "algorithm_formatter.hpp",
        ],
        srcs = [
                "instance.cpp",
                "instance_builder.cpp",
                "solution.cpp",
                "reduction.cpp",
                "algorithm.cpp",
                "algorithm_formatter.cpp",
        ],
        deps = [
                "@optimizationtools//optimizationtools/utils:utils",
                "@optimizationtools//optimizationtools/containers:containers",
        ],
        visibility = ["//visibility:public"],
)

cc_binary(
        name = "main",
        srcs = ["main.cpp"],
        deps = [
                "//stablesolver/stable/algorithms:greedy",
                "//stablesolver/stable/algorithms:milp_cbc",
                "//stablesolver/stable/algorithms:milp_cplex",
                "//stablesolver/stable/algorithms:local_search",
                "//stablesolver/stable/algorithms:local_search_row_weighting",
                "//stablesolver/stable/algorithms:large_neighborhood_search",
                "@boost//:program_options",
        ],
        linkopts = select({
                    "@bazel_tools//src/conditions:windows": [],
                    "//conditions:default": ["-lpthread"],
        }),
)
