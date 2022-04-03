config_setting(name = "cplex_build", values = {"define": "cplex=true"}, visibility = ["//visibility:public"])

load("//stablesolver:variables.bzl", "STDCPP")

cc_library(
        name = "cliquesolver",
        hdrs = [
                "instance.hpp",
                "solution.hpp",
        ],
        srcs = [
                "instance.cpp",
                "solution.cpp",
        ],
        deps = [
                "@optimizationtools//optimizationtools/utils:info",
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
                "//cliquesolver/algorithms:algorithms",
        ],
        linkopts = select({
                    "@bazel_tools//src/conditions:windows": [],
                    "//conditions:default":                 ["-lpthread"],
        }),
)

