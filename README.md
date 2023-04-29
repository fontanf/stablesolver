# Stable Solver and Clique Solver

Solvers for the Maximum(-Weight) Independent Set and for the Maximum(-Weight) Clique Problems.

![stable](stable.png?raw=true "stable")

[image source](https://commons.wikimedia.org/wiki/File:Independent_set_graph.svg)

## Implemented algorithms

To solve a stable (resp. clique) problem, it is possible to use a clique (resp. stable) algorithm on the complementary graph (option `--complementary`). However, graphs being generally sparse, the complementary graph might be huge. When a more optimized implementation is possible, both are implemented.

The stable solver can also be used to solve the Minimum (Weight) Vertex Cover Problem by just considering the vertices outside of the solution.

### Stable Solver

- Greedy algorithms, see "A note on greedy algorithms for the maximum weighted independent set problem" (Sakai et al., 2001) [DOI](https://doi.org/10.1016/S0166-218X(02)00205-6)
  - `-a greedy_gwmin`
  - `-a greedy_gwmax`
  - `-a greedy_gwmin2`
  - `-a greedy_strong`

- Mixed-Integer Linear Programs
  - Model 1, `|E|` constraints  `-a milp_1_cbc` (Cbc) `-a milp_1_cplex` (CPLEX)
  - Model 2, `|V|` constraints, see "A multi-KP modeling for the maximum-clique problem" (Della Croce et Tadei, 1994) [DOI](https://doi.org/10.1016/0377-2217(94)90252-6) `-a milp_2_cplex` (CPLEX)
  - Model 3, clique constraints, see "A Branch-and-Bound Algorithm for the Knapsack Problem with Conflict Graph" (Bettinelli et al., 2017) [DOI](https://doi.org/10.1287/ijoc.2016.0742) (seems useless since solvers already detect and merge clique constraints) `-a milp_3_cplex` (CPLEX)

- Local search algorithm implemented with [fontanf/localsearchsolver](https://github.com/fontanf/localsearchsolver) `-a "localsearch --threads 3"`

- Row weighting local search (unweighted only)
  - based on "Weighting-Based Parallel Local Search for Optimal Camera Placement and Unicost Set Covering" (Lin et al., 2020) [DOI](https://doi.org/10.1145/3377929.3398184) `-a "localsearch_rowweighting_1 --iteration-limit 100000 --iteration-without-improvment-limit 10000"`
  - based on "An efficient local search heuristic with row weighting for the unicost set covering problem" (Gao et al., 2015) [DOI](https://doi.org/10.1016/j.ejor.2015.05.038) `-a "localsearch_rowweighting_2 --iteration-limit 100000 --iteration-without-improvment-limit 10000"`

- Large neighborhoodsearch based on "NuMWVC: A novel local search for minimum weighted vertex cover problem" (Li et al., 2020) [DOI](https://doi.org/10.1080/01605682.2019.1621218) `-a "largeneighborhoodsearch"`

### Clique Solver

- Greedy algorithms:
  - `-a greedy_gwmin`, adapted from the stable version, same complexity
  - `-a greedy_strong`

- Mixed-Integer Linear Program (implemented with CPLEX), see "Worst-case analysis of clique MIPs" (Naderi et al., 2021) [DOI](https://doi.org/10.1007/s10107-021-01706-2) `-a milp_cplex`

- Local search algorithm implemented with [fontanf/localsearchsolver](https://github.com/fontanf/localsearchsolver) `-a "localsearch"`

## Usage (command line)

Download and uncompress the instances in the `data/` folder:


Compile:
```shell
bazel build -- //...
```

Examples:

```shell
./bazel-bin/stablesolver/main -v 1 -i "data/dimacs1992/brock200_1.clq" --format dimacs1992 -a "localsearch_rowweighting_2 --iterations 3000" -c solution.txt
```
```
=====================================
            Stable Solver            
=====================================

Instance
--------
Number of vertices:              200
Number of edges:                 5066
Density:                         0.254573
Average degree:                  50.66
Maximum degree:                  69
Average weight:                  1
Number of connected components:  1

Algorithm
---------
Row Weighting Local Search 1

       T (s)              LB              UB             GAP     GAP (%)                 Comment
       -----              --              --             ---     -------                 -------
       0.000               0             200             200      100.00                        
       0.000              15             200             185       92.50        initial solution
       0.000              16             200             184       92.00             iteration 2
       0.007              17             200             183       91.50             iteration 2
       0.007              18             200             182       91.00             iteration 2
       0.007              19             200             181       90.50             iteration 3
       0.008              20             200             180       90.00          iteration 1162
       0.010              21             200             179       89.50          iteration 2440

Final statistics
----------------
Value:                 21
Number of vertices:    21
Vertex cover Value:    179
Bound:                 200
Gap:                   179
Gap (%):               89.5
Time (s):              0.0103
Number of iterations:  3000
```

```shell
./bazel-bin/stablesolver/main -v 1 -i "data/dimacs2010/clustering/caidaRouterLevel.graph" -f dimacs2010 --reduce -a "localsearch_rowweighting_1 --iterations 300000"
```
```
=====================================
            Stable Solver            
=====================================

Instance
--------
Number of vertices:              192244
Number of edges:                 609066
Density:                         3.29603e-05
Average degree:                  6.33639
Maximum degree:                  1071
Average weight:                  1
Number of connected components:  308

Reduced instance
----------------
Number of vertices:              2800
Number of edges:                 8646
Density:                         0.0022064
Average degree:                  6.17571
Maximum degree:                  56
Extra weight:                    115593
Number of connected components:  148

Algorithm
---------
Row Weighting Local Search 1

Parameters
----------
Maximum number of iterations:                      300000
Maximum number of iterations without improvement:  -1

       T (s)              LB              UB             GAP     GAP (%)                 Comment
       -----              --              --             ---     -------                 -------
       0.425               0          118393          118393      100.00                        
       0.441          117029          118393            1364        1.15        initial solution
       0.815          117146          118393            1247        1.05        iteration 100000
       1.207          117150          118393            1243        1.05        iteration 200000

Final statistics
----------------
Value:                 117150
Number of vertices:    117150
Vertex cover Value:    75094
Bound:                 118393
Gap:                   1243
Gap (%):               1.04989
Time (s):              1.5777
Number of iterations:  300000
```
