STDCPP = select({
            "@bazel_tools//src/conditions:windows": ['/std:c++latest'],
            "//conditions:default":                 ["-std=c++14"],})

COINOR_COPTS = select({
            "//stablesolver:coinor_build": ["-DCOINOR_FOUND"],
            "//conditions:default":        []})
CPLEX_COPTS = select({
            "//stablesolver:cplex_build": [
                    "-DCPLEX_FOUND",
                    "-m64",
                    "-DIL_STD"],
            "//conditions:default": []})

COINOR_DEP = select({
            "//stablesolver:coinor_build": ["@coinor//:coinor"],
            "//conditions:default": []})
CPLEX_DEP = select({
            "//stablesolver:cplex_build": ["@cplex//:cplex"],
            "//conditions:default": []})
CPOPTIMIZER_DEP = select({
            "//stablesolver:cplex_build": ["@cplex//:cpoptimizer"],
            "//conditions:default": []})

