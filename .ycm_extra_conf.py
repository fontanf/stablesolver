def Settings( **kwargs ):
    return {
            'flags': [
                '-x', 'c++',
                '-Wall', '-Wextra', '-Werror',
                '-DCPLEX_FOUND',
                '-DCOINOR_FOUND',
                '-DIL_STD', # Cplex
                '-I', '.',
                '-I', './bazel-stablesolver/external/json/single_include',
                '-I', './bazel-stablesolver/external/googletest/googletest/include',
                '-I', './bazel-stablesolver/external/boost/',
                '-I', './bazel-stablesolver/external/optimizationtools',
                # '-I', '../optimizationtools',
                '-I', './bazel-stablesolver/external/localsearchsolver/',
                # COINOR
                '-I', '/home/florian/Programmes/coinbrew/dist/include/',
                # CPLEX
                '-I', '/opt/ibm/ILOG/CPLEX_Studio129/concert/include/',
                '-I', '/opt/ibm/ILOG/CPLEX_Studio129/cplex/include/',
                '-I', '/opt/ibm/ILOG/CPLEX_Studio129/cpoptimizer/include/',
                ],
            }

