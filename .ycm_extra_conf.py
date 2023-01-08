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

                # COINOR
                '-DCOINOR_FOUND',
                '-I', '/home/florian/Programmes/coinbrew/dist/include/',

                # CPLEX
                '-DCPLEX_FOUND',
                '-DIL_STD',
                '-I', '/opt/ibm/ILOG/CPLEX_Studio129/concert/include/',
                '-I', '/opt/ibm/ILOG/CPLEX_Studio129/cplex/include/',
                '-I', '/opt/ibm/ILOG/CPLEX_Studio129/cpoptimizer/include/',

                ],
            }
