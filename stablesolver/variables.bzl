STDCPP = select({
            "@bazel_tools//src/conditions:windows": ['/std:c++latest'],
            "//conditions:default": ["-std=c++14"],})

CBC_COPTS = select({
            "//stablesolver:cbc_build": ["-DCBC_FOUND"],
            "//conditions:default": []})
CBC_DEP = select({
            "//stablesolver:cbc_windows": ["@cbc_windows//:cbc"],
            "//conditions:default": []
        }) + select({
            "//stablesolver:cbc_linux": ["@cbc_linux//:cbc"],
            "//conditions:default": []})

CPLEX_COPTS = select({
            "//stablesolver:cplex_build": [
                    "-DCPLEX_FOUND",
                    "-m64",
                    "-DIL_STD"],
            "//conditions:default": []})
CPLEX_DEP = select({
            "//stablesolver:cplex_build": ["@cplex//:cplex"],
            "//conditions:default": []})

XPRESS_COPTS = select({
            "//stablesolver:xpress_build": ["-DXPRESS_FOUND"],
            "//conditions:default": []})
XPRESS_DEP = select({
            "//stablesolver:xpress_build": ["@xpress//:xpress"],
            "//conditions:default": []})

ALL_COPTS = CBC_COPTS + XPRESS_COPTS + CPLEX_COPTS
ALL_DEP = CBC_DEP + XPRESS_DEP + CPLEX_DEP

