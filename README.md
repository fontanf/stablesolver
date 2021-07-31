# Stable Solver and Clique Solver

WORK IN PROGRESS

Solvers for the Maximum(-Weight) Independent Set and for the Maximum(-Weight) Clique Problems.

![stable](stable.png?raw=true "stable")

[image source](https://commons.wikimedia.org/wiki/File:Independent_set_graph.svg)

## Implemented algorithms

To solve a stable (resp. clique) problem, it is possible to use a clique (resp. stable) algorithm on the complementary graph (option `--complementary`). However, graphs being generally sparse, the complementary graph might be huge. When a more optimized implementation is possible, both are implemented.

The stable solver can also be used to solve the Minimum (Weight) Vertex Cover Problem by just considering the vertices outside of the solution.

### Stable Solver

Greedy algorithms, see "A note on greedy algorithms for the maximum weighted independent set problem" (Sakai et al., 2001) [DOI](https://doi.org/10.1016/S0166-218X(02)00205-6)
* `-a greedy_gwmin`
* `-a greedy_gwmax`
* `-a greedy_gwmin2`

Mixed-Integer Linear Programs (implemented with CPLEX)
* Model 1, `|E|` constraints `-a milp_1_cplex`
* Model 2, `|V|` constraints, see "A multi-KP modeling for the maximum-clique problem" (Della Croce et Tadei, 1994) [DOI](https://doi.org/10.1016/0377-2217(94)90252-6) `-a milp_2_cplex`
* Model 3, clique constraints, see "A Branch-and-Bound Algorithm for the Knapsack Problem with Conflict Graph" (Bettinelli et al., 2017) [DOI](https://doi.org/10.1287/ijoc.2016.0742) (seems useless since solvers already detect and merge clique constraints) `-a milp_3_cplex`

Local search algorithm implemented with [fontanf/localsearchsolver](https://github.com/fontanf/localsearchsolver) `-a "localsearch --threads 3"`

Row weighting local search (unweighted only)
* based on "Weighting-Based Parallel Local Search for Optimal Camera Placement and Unicost Set Covering" (Lin et al., 2020) `-a "localsearch_rowweighting_1 --threads 4 --iteration-limit 100000 --iteration-without-improvment-limit 10000"`
* based on "An efficient local search heuristic with row weighting for the unicost set covering problem" (Gao et al., 2015) `-a "localsearch_rowweighting_2 --threads 4 --iteration-limit 100000 --iteration-without-improvment-limit 10000"`

Large neighborhoodsearch based on "NuMWVC: A novel local search for minimum weighted vertex cover problem" (Li et al., 2020) `-a "largeneighborhoodsearch"`

### Clique Solver

Greedy algorithms:
* `-a greedy_gwmin`, adapted from the stable version, same complexity

## Usage (command line)

Download and uncompress the instances in the `data/` folder:


Compile:
```shell
bazel build -- //...
```

Run:
```shell
./bazel-bin/stablesolver/main -v -i "data/graphstable/1-FullIns_3.col" -a greedy_dsatur -c solution.txt
./bazel-bin/stablesolver/main -v -i "data/graphstable/r1000.5.col" -a "localsearch --threads 3"
./bazel-bin/stablesolver/main -v -i "data/graphstable/1-FullIns_3.col" -a milp_assignment_cplex -t 60 -c solution.txt
```

