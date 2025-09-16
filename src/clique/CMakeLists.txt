add_library(StableSolver_clique)
target_sources(StableSolver_clique PRIVATE
    instance.cpp
    solution.cpp
    algorithm_formatter.cpp)
target_include_directories(StableSolver_clique PUBLIC
    ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(StableSolver_clique PUBLIC
    OptimizationTools::utils
    OptimizationTools::containers
    OptimizationTools::graph)
add_library(StableSolver::clique ALIAS StableSolver_clique)

add_subdirectory(algorithms)

add_executable(StableSolver_clique_main)
target_sources(StableSolver_clique_main PRIVATE
    main.cpp)
target_link_libraries(StableSolver_clique_main PUBLIC
    StableSolver_clique_greedy
    StableSolver_clique_milp
    StableSolver_clique_local_search
    Boost::program_options)
set_target_properties(StableSolver_clique_main PROPERTIES OUTPUT_NAME "stablesolver_clique")
install(TARGETS StableSolver_clique_main)
