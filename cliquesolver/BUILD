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
                "//stablesolver:stablesolver",
                "@optimizationtools//optimizationtools:info",
                "@optimizationtools//optimizationtools:utils",
                "@optimizationtools//optimizationtools:indexed_collections",
        ],
        visibility = ["//visibility:public"],
)

cc_binary(
        name = "main",
        srcs = ["main.cpp"],
        deps = [
                "//cliquesolver/algorithms:algorithms",
                "@boost//:program_options",
        ],
        linkopts = select({
                    "@bazel_tools//src/conditions:windows": [],
                    "//conditions:default":                 ["-lpthread"],
        }),
)

