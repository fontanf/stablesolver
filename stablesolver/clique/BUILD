load("//stablesolver:variables.bzl", "STDCPP")

cc_library(
        name = "clique",
        hdrs = [
                "instance.hpp",
                "solution.hpp",
                "algorithm_formatter.hpp",
        ],
        srcs = [
                "instance.cpp",
                "solution.cpp",
                "algorithm_formatter.cpp",
        ],
        deps = [
                "@optimizationtools//optimizationtools/utils:utils",
                "@optimizationtools//optimizationtools/containers:containers",
                "@optimizationtools//optimizationtools/graph:graph",
        ],
        visibility = ["//visibility:public"],
)

cc_binary(
        name = "main",
        srcs = ["main.cpp"],
        deps = [
                "//stablesolver/clique/algorithms:greedy",
                "//stablesolver/clique/algorithms:milp_cplex",
                "//stablesolver/clique/algorithms:local_search",
                "@boost//:program_options",
        ],
        linkopts = select({
                    "@bazel_tools//src/conditions:windows": [],
                    "//conditions:default": ["-lpthread"],
        }),
)
