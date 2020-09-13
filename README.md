# StableSolver and CliqueSolver

Solvers for the Maximum(-Weight) Independent Set and for the Maximum(-Weight) Clique Problems.

![knapsack](stable.png?raw=true "stable")

[image source](https://commons.wikimedia.org/wiki/File:Independent_set_graph.svg)

## Implemented algorithms

To solve a stable (resp. clique) problem, it is possible to use a clique (resp. stable) algorithm on the complementary graph (option `--complementary`). However, gaphs being generally sparse, the complementary graph might be huge. When a more optimized implementation is possible, both are implemented.

StableSolver:

* Greedy algorithms, see "A note on greedy algorithms for the maximum weighted independent set problem" (Sakai et al., 2001):
  * `-a greedy_gwmin`
  * `-a greedy_gwmax`
  * `-a greedy_gwmin2`
* Branch-and-cut (CPLEX)
  * Model 1  `-a branchandcut_1_cplex`, `|E|` constraints
  * Model 2  `-a branchandcut_2_cplex`, `|V|` constraints, see "A multi-KP modeling for the maximum-clique problem" (Della Croce et Tadei, 1994)
  * Model 3  `-a branchandcut_3_cplex`, clique constraints, see "A Branch-and-Bound Algorithm for the Knapsack Problem with Conflict Graph" (Bettinelli et al., 2017)

CliqueSolver:

* Greedy algorithms:
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
./bazel-bin/stablesolver/main -v -i "data/graphstable/1-FullIns_3.col" -a branchandcut_assignment_cplex -t 60 -c solution.txt
```

