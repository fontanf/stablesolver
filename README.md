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
  - Model 1, `|E|` constraints  `-a milp-1`
  - Model 2, `|V|` constraints, see "A multi-KP modeling for the maximum-clique problem" (Della Croce et Tadei, 1994) [DOI](https://doi.org/10.1016/0377-2217(94)90252-6) `-a milp-2`
  - Model 3, clique constraints, see "A Branch-and-Bound Algorithm for the Knapsack Problem with Conflict Graph" (Bettinelli et al., 2017) [DOI](https://doi.org/10.1287/ijoc.2016.0742) (seems useless since solvers already detect and merge clique constraints) `-a milp-3`

- Local search algorithm implemented with [fontanf/localsearchsolver](https://github.com/fontanf/localsearchsolver) `-a "local-search --threads 3"`

- Row weighting local search (unweighted only), the row weighting local search of [fontanf/setcoveringsolver](https://github.com/fontanf/setcoveringsolver) applied to the minimum vertex cover problem `-a local-search-row-weighting`

- Large neighborhoodsearch based on "NuMWVC: A novel local search for minimum weighted vertex cover problem" (Li et al., 2020) [DOI](https://doi.org/10.1080/01605682.2019.1621218) `-a "large-neighborhood-search"`

### Clique

- Greedy algorithms:
  - `-a greedy-gwmin`, adapted from the stable version, same complexity
  - `-a greedy-strong`

- Mixed-Integer Linear Program from "Worst-case analysis of clique MIPs" (Naderi et al., 2021) [DOI](https://doi.org/10.1007/s10107-021-01706-2) `-a milp`

- Local search algorithm implemented with [fontanf/localsearchsolver](https://github.com/fontanf/localsearchsolver) `-a "local-search"`

## Usage (command line)

Download and uncompress the instances in the `data/` folder:


Compile:
```shell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
cmake --install build --config Release --prefix install
```

Setup Python environment to use the Python scripts:
```shell
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

Download data:
```shell
python3 scripts/download_data.py
```

Examples:

```shell
./install/bin/stablesolver_stable  --verbosity-level 1  --input "data/dimacs1992/brock200_1.clq" --format dimacs1992  --algorithm "local-search-row-weighting" --maximum-number-of-iterations 3000  --certificate solution.txt
```
```
=====================================
            StableSolver            
=====================================

Problem
-------
Maximum(-weight) independent set problem

Instance
--------
Number of vertices:              200
Number of edges:                 5066
Density:                         0.2533
Average degree:                  50.66
Highest degree:                  69
Total weight:                    200
Number of connected components:  1

Algorithm
---------
Row weighting local search

Parameters
----------
Time limit:                          inf
Messages
    Verbosity level:                 1
    Standard output:                 1
    File path:                       
    # streams:                       0
Logger
    Has logger:                      0
    Standard error:                  0
    File path:                       
Reduction
    Enable:                          1
    Max. # of rounds:                10
Max. # of iterations:                3000
Max. # of iterations without impr.:  -1
Best solution update frequency:      100000

Reduced instance
----------------
Number of vertices:              200
Number of edges:                 5066
Density:                         0.2533
Average degree:                  50.66
Highest degree:                  69
Total weight:                    200
Number of connected components:  1

    Time (s)       Value       Bound         Gap     Gap (%)                 Comment
    --------       -----       -----         ---     -------                 -------
       0.000           0         200         200      100.00                        
       0.002          15         200         185       92.50        initial solution
       0.008          20         200         180       90.00                 it 3000

Final statistics
----------------
Value:                         20
Bound:                         200
Absolute optimality gap:       180
Relative optimality gap (%):   90
Time (s):                      0.00832587
Number of iterations:          3000

Solution
--------
Number of vertices:   20 / 200 (10%)
Number of conflicts:  0
Feasible:             1
Vertex cover weight:  180
Weight:               20
```

```shell
./install/bin/stablesolver_stable  --verbosity-level 1  --input "data/networkrepository/inf-roadNet-PA.mtx" --format matrixmarket  --algorithm "local-search-row-weighting" --maximum-number-of-iterations 1000000
```
```
=====================================
            StableSolver            
=====================================

Problem
-------
Maximum(-weight) independent set problem

Instance
--------
Number of vertices:              1087562
Number of edges:                 1541514
Density:                         2.60657e-06
Average degree:                  2.83481
Highest degree:                  9
Total weight:                    1087562
Number of connected components:  1

Algorithm
---------
Row weighting local search

Parameters
----------
Time limit:                          inf
Messages
    Verbosity level:                 1
    Standard output:                 1
    File path:                       
    # streams:                       0
Logger
    Has logger:                      0
    Standard error:                  0
    File path:                       
Reduction
    Enable:                          1
    Max. # of rounds:                10
Max. # of iterations:                1000000
Max. # of iterations without impr.:  -1
Best solution update frequency:      100000

Reduced instance
----------------
Number of vertices:              49023
Number of edges:                 86253
Density:                         7.17802e-05
Average degree:                  3.51888
Highest degree:                  12
Total weight:                    49023
Number of connected components:  366

    Time (s)       Value       Bound         Gap     Gap (%)                 Comment
    --------       -----       -----         ---     -------                 -------
       0.000           0     1087562 1.08756e+06      100.00                        
       4.465      510762     1087562      576800       53.04                        
       4.466      510762      545907       35145        6.44                        
       4.524      530013      545907       15894        2.91        initial solution
       4.640      533178      545907       12729        2.33               it 100000
       4.747      533262      545907       12645        2.32               it 200000
       4.851      533272      545907       12635        2.31               it 300000
       4.955      533279      545907       12628        2.31               it 400000
       5.083      533291      545907       12616        2.31               it 500000
       5.236      533298      545907       12609        2.31               it 600000
       5.400      533305      545907       12602        2.31               it 700000
       5.563      533306      545907       12601        2.31               it 800000
       5.727      533312      545907       12595        2.31               it 900000
       5.886      533320      545907       12587        2.31              it 1000000

Final statistics
----------------
Value:                         533320
Bound:                         545907
Absolute optimality gap:       12587
Relative optimality gap (%):   2.3057
Time (s):                      5.88743
Number of iterations:          1000000

Solution
--------
Number of vertices:   533320 / 1087562 (49.0381%)
Number of conflicts:  0
Feasible:             1
Vertex cover weight:  554242
Weight:               533320
```
