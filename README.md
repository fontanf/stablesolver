# StableSolver and CliqueSolver

Solvers for the Maximum(-Weight) Independent Set and for the Maximum(-Weight) Clique Problems.

![knapsack](stable.png?raw=true "stable")

[image source](https://commons.wikimedia.org/wiki/File:Independent_set_graph.svg)

## Implemented algorithms

StableSolver:

* Greedy algorithms, see "A note on greedy algorithms for the maximum weighted independent set problem" (Sakai et al., 2001) for their descriptions:
  * `-a greedy_gwmin`
  * `-a greedy_gwmax`
  * `-a greedy_gwmin2`
* Branch-and-cut (CPLEX) `-a branchandcut_cplex`

CliqueSolver:

* Greedy algorithms:
  * `-a greedy_gwmin`, adapted from the stable version, same complexity
* Branch-and-cut (CPLEX) `-a branchandcut_cplex`, the number of constraints is the number of non-edge, which can be very high for sparse graphs

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

