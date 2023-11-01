def Settings(**kwargs):
    return {
            'flags': [
                '-x', 'c++',
                '-Wall', '-Wextra', '-Werror',
                '-I', '.',

                '-I', './bazel-stablesolver/external/'
                'json/single_include/',

                '-I', './bazel-stablesolver/external/'
                'googletest/googletest/include/',

                '-I', './bazel-stablesolver/external/'
                'boost/',

                # optimizationtools
                '-I', './bazel-stablesolver/external/'
                # '-I', './../'
                'optimizationtools/',

                # localsearchsolver
                '-I', './bazel-stablesolver/external/'
                # '-I', './../',
                'localsearchsolver/',

                # CBC
                '-DCBC_FOUND',
                '-I', './bazel-stablesolver/external/cbc_linux/include/coin/',

                # CPLEX
                '-DCPLEX_FOUND',
                '-DIL_STD',
                '-I', '/opt/ibm/ILOG/CPLEX_Studio129/concert/include/',
                '-I', '/opt/ibm/ILOG/CPLEX_Studio129/cplex/include/',
                '-I', '/opt/ibm/ILOG/CPLEX_Studio129/cpoptimizer/include/',

                ],
            }
