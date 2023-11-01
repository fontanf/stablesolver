# StableSolver

A solver for the maximum(-weight) independent set and for the maximum(-weight) clique problems.

![stable](stable.png?raw=true "stable")

[image source](https://commons.wikimedia.org/wiki/File:Independent_set_graph.svg)

## Implemented algorithms

To solve a stable (resp. clique) problem, it is possible to use a clique (resp. stable) algorithm on the complementary graph (option `--complementary`). However, graphs being generally sparse, the complementary graph might be huge. When a more optimized implementation is possible, both are implemented.

The stable solver can also be used to solve the Minimum (Weight) Vertex Cover Problem by just considering the vertices outside of the solution.

### Stable

- Greedy algorithms, see "A note on greedy algorithms for the maximum weighted independent set problem" (Sakai et al., 2001) [DOI](https://doi.org/10.1016/S0166-218X(02)00205-6)
  - `-a greedy-gwmin`
  - `-a greedy-gwmax`
  - `-a greedy-gwmin2`
  - `-a greedy-strong`

- Mixed-Integer Linear Programs
  - Model 1, `|E|` constraints  `-a milp-1-cbc` (Cbc) `-a milp-1-cplex` (CPLEX)
  - Model 2, `|V|` constraints, see "A multi-KP modeling for the maximum-clique problem" (Della Croce et Tadei, 1994) [DOI](https://doi.org/10.1016/0377-2217(94)90252-6) `-a milp-2-cplex` (CPLEX)
  - Model 3, clique constraints, see "A Branch-and-Bound Algorithm for the Knapsack Problem with Conflict Graph" (Bettinelli et al., 2017) [DOI](https://doi.org/10.1287/ijoc.2016.0742) (seems useless since solvers already detect and merge clique constraints) `-a milp-3-cplex` (CPLEX)

- Local search algorithm implemented with [fontanf/localsearchsolver](https://github.com/fontanf/localsearchsolver) `-a "local-search --threads 3"`

- Row weighting local search (unweighted only)
  - based on "Weighting-Based Parallel Local Search for Optimal Camera Placement and Unicost Set Covering" (Lin et al., 2020) [DOI](https://doi.org/10.1145/3377929.3398184) `-a "local-search-row-weighting-1 --iteration-limit 100000 --iteration-without-improvment-limit 10000"`
  - based on "An efficient local search heuristic with row weighting for the unicost set covering problem" (Gao et al., 2015) [DOI](https://doi.org/10.1016/j.ejor.2015.05.038) `-a "local-search-row-weighting-2 --iteration-limit 100000 --iteration-without-improvment-limit 10000"`

- Large neighborhoodsearch based on "NuMWVC: A novel local search for minimum weighted vertex cover problem" (Li et al., 2020) [DOI](https://doi.org/10.1080/01605682.2019.1621218) `-a "large-neighborhood-search"`

### Clique

- Greedy algorithms:
  - `-a greedy-gwmin`, adapted from the stable version, same complexity
  - `-a greedy-strong`

- Mixed-Integer Linear Program (implemented with CPLEX), see "Worst-case analysis of clique MIPs" (Naderi et al., 2021) [DOI](https://doi.org/10.1007/s10107-021-01706-2) `-a milp-cplex`

- Local search algorithm implemented with [fontanf/localsearchsolver](https://github.com/fontanf/localsearchsolver) `-a "local-search"`

## Usage (command line)

Download and uncompress the instances in the `data/` folder:


Compile:
```shell
bazel build -- //...
```

Examples:

```shell
./bazel-bin/stablesolver/main -v 1 -i "data/dimacs1992/brock200_1.clq" --format dimacs1992 -a "local-search-row-weighting-2 --iterations 3000" -c solution.txt
```
```
====================================
            StableSolver            
====================================

Problem
-------
Maximum-weight independent set problem

Instance
--------
Number of vertices:              200
Number of edges:                 5066
Density:                         0.2533
Average degree:                  50.66
Maximum degree:                  69
Total weight:                    200
Number of connected components:  1

Algorithm
---------
Row weighting local search 2

Reduced instance
----------------
Number of vertices:              200
Number of edges:                 5066
Density:                         0.2533
Average degree:                  50.66
Maximum degree:                  69
Total weight:                    200
Number of connected components:  1

       T (s)              LB              UB             GAP     GAP (%)                 Comment
       -----              --              --             ---     -------                 -------
       0.003               0             200             200      100.00                        
       0.003              15             200             185       92.50        initial solution
       0.003              16             200             184       92.00             iteration 2
       0.010              17             200             183       91.50             iteration 2
       0.010              18             200             182       91.00             iteration 2
       0.010              19             200             181       90.50             iteration 3
       0.012              20             200             180       90.00          iteration 1162
       0.013              21             200             179       89.50          iteration 2440

Final statistics
----------------
Value:                        21
Bound:                        200
Absolute optimality gap:      179
Relative optimality gap (%):  89.5
Time (s):                     0.0134117
Number of iterations:         3000

Solution
--------
Number of vertices:   21 / 200 (10.5%)
Number of conflicts:  0
Feasible:             1
Vertex cover weight:  179
Weight:               21
```

```shell
./bazel-bin/stablesolver/main -v 1 -i "data/dimacs2010/clustering/caidaRouterLevel.graph" -f dimacs2010 -a "local-search-row-weighting-1 --iterations 300000"
```
```
====================================
            StableSolver            
====================================

Problem
-------
Maximum-weight independent set problem

Instance
--------
Number of vertices:              192244
Number of edges:                 609066
Density:                         3.29601e-05
Average degree:                  6.33639
Maximum degree:                  1071
Total weight:                    192244
Number of connected components:  308

Algorithm
---------
Row weighting local search 1

Parameters
----------
Maximum number of iterations:                      300000
Maximum number of iterations without improvement:  -1

Reduced instance
----------------
Number of vertices:              2800
Number of edges:                 8646
Density:                         0.00220561
Average degree:                  6.17571
Maximum degree:                  56
Total weight:                    2800
Number of connected components:  148

       T (s)              LB              UB             GAP     GAP (%)                 Comment
       -----              --              --             ---     -------                 -------
       0.505               0          192244          192244      100.00                        
       0.505               0          118393          118393      100.00               reduction
       0.522          117029          118393            1364        1.15        initial solution
       0.827          117146          118393            1247        1.05        iteration 100000
       1.141          117150          118393            1243        1.05        iteration 200000

Final statistics
----------------
Value:                        117150
Bound:                        118393
Absolute optimality gap:      1243
Relative optimality gap (%):  1.04989
Time (s):                     1.42409
Number of iterations:         300000

Solution
--------
Number of vertices:   117150 / 192244 (60.9382%)
Number of conflicts:  0
Feasible:             1
Vertex cover weight:  75094
Weight:               117150
```
