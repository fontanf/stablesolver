add_library(StableSolver_stable_greedy)
target_sources(StableSolver_stable_greedy PRIVATE
    greedy.cpp)
target_include_directories(StableSolver_stable_greedy PUBLIC
    ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(StableSolver_stable_greedy PUBLIC
    StableSolver_stable)
add_library(StableSolver::stable::greedy ALIAS StableSolver_stable_greedy)

add_library(StableSolver_stable_milp)
target_sources(StableSolver_stable_milp PRIVATE
    milp.cpp)
target_include_directories(StableSolver_stable_milp PUBLIC
    ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(StableSolver_stable_milp PUBLIC
    StableSolver_stable
    StableSolver_clique_greedy
    MathOptSolversCMake::milp)
add_library(StableSolver::stable::milp ALIAS StableSolver_stable_milp)

add_library(StableSolver_stable_local_search)
target_sources(StableSolver_stable_local_search PRIVATE
    local_search.cpp)
target_include_directories(StableSolver_stable_local_search PUBLIC
    ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(StableSolver_stable_local_search PUBLIC
    StableSolver_stable
    LocalSearchSolver::localsearchsolver)
add_library(StableSolver::stable::local_search ALIAS StableSolver_stable_local_search)

add_library(StableSolver_stable_local_search_row_weighting)
target_sources(StableSolver_stable_local_search_row_weighting PRIVATE
    local_search_row_weighting.cpp)
target_include_directories(StableSolver_stable_local_search_row_weighting PUBLIC
    ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(StableSolver_stable_local_search_row_weighting PUBLIC
    StableSolver_stable
    StableSolver_stable_greedy)
add_library(StableSolver::stable::local_search_row_weighting ALIAS StableSolver_stable_local_search_row_weighting)

add_library(StableSolver_stable_large_neighborhood_search)
target_sources(StableSolver_stable_large_neighborhood_search PRIVATE
    large_neighborhood_search.cpp)
target_include_directories(StableSolver_stable_large_neighborhood_search PUBLIC
    ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(StableSolver_stable_large_neighborhood_search PUBLIC
    StableSolver_stable)
add_library(StableSolver::stable::large_neighborhood_search ALIAS StableSolver_stable_large_neighborhood_search)
