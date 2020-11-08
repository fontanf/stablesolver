# Stable Solver and Clique Solver

Solvers for the Maximum(-Weight) Independent Set and for the Maximum(-Weight) Clique Problems.

![stable](stable.png?raw=true "stable")

[image source](https://commons.wikimedia.org/wiki/File:Independent_set_graph.svg)

## Implemented algorithms

To solve a stable (resp. clique) problem, it is possible to use a clique (resp. stable) algorithm on the complementary graph (option `--complementary`). However, gaphs being generally sparse, the complementary graph might be huge. When a more optimized implementation is possible, both are implemented.

The stable solver can also be used to solve the Minimum (Weight) Vertex Cover Problem by just considering the vertices outside of the solution.

Stable Solver:

* Greedy algorithms, see "A note on greedy algorithms for the maximum weighted independent set problem" (Sakai et al., 2001):
  * `-a greedy_gwmin`
  * `-a greedy_gwmax`
  * `-a greedy_gwmin2`
* Branch-and-cut (CPLEX)
  * Model 1, `|E|` constraints `-a branchandcut_1_cplex`
  * Model 2, `|V|` constraints, see "A multi-KP modeling for the maximum-clique problem" (Della Croce et Tadei, 1994) `-a branchandcut_2_cplex`
  * Model 3, clique constraints, see "A Branch-and-Bound Algorithm for the Knapsack Problem with Conflict Graph" (Bettinelli et al., 2017) (seems useless since solvers already detect and merge clique constraints) `-a branchandcut_3_cplex`
* Decision diagram
  * Restricted decision diagram (solution) `-a "decisiondiagram_restricted --width 100"` :x:
  * Relaxed decision diagram (bound) `-a "decisiondiagram_relaxed --width 100"` :x:
  * Decision Diagrams by Separation (bound and solution at the end) `-a "decisiondiagram_separation"` :x:
  * Decision diagram based branch-and-bound `-a "branchandbound_decisiondiagram"` :x:
* Row weighting local search (unweighted only)
  * `-a "localsearch_1 --threads 3 --iterations 10000"`
  * `-a "localsearch_2 --threads 3 --iterations 10000"`
* Large neighborhoodsearch based on "NuMWVC: A novel local search for minimum weighted vertex cover problem" (Li et al., 2020) `-a "largeneighborhoodsearch"`

Clique Solver:

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

