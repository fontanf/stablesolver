add_library(StableSolver_clique_greedy)
target_sources(StableSolver_clique_greedy PRIVATE
    greedy.cpp)
target_include_directories(StableSolver_clique_greedy PUBLIC
    ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(StableSolver_clique_greedy PUBLIC
    StableSolver_clique)
add_library(StableSolver::clique::greedy ALIAS StableSolver_clique_greedy)

add_library(StableSolver_clique_milp)
target_sources(StableSolver_clique_milp PRIVATE
    milp.cpp)
target_include_directories(StableSolver_clique_milp PUBLIC
    ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(StableSolver_clique_milp PUBLIC
    StableSolver_clique
    MathOptSolversCMake::milp)
add_library(StableSolver::clique::milp ALIAS StableSolver_clique_milp)

add_library(StableSolver_clique_local_search)
target_sources(StableSolver_clique_local_search PRIVATE
    local_search.cpp)
target_include_directories(StableSolver_clique_local_search PUBLIC
    ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(StableSolver_clique_local_search PUBLIC
    StableSolver_clique
    LocalSearchSolver::localsearchsolver)
add_library(StableSolver::clique::local_search ALIAS StableSolver_clique_local_search)
