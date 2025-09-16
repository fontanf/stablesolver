add_library(StableSolver_stable)
target_sources(StableSolver_stable PRIVATE
    instance.cpp
    instance_builder.cpp
    solution.cpp
    reduction.cpp
    algorithm.cpp
    algorithm_formatter.cpp)
target_include_directories(StableSolver_stable PUBLIC
    ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(StableSolver_stable PUBLIC
    OptimizationTools::utils
    OptimizationTools::containers)
add_library(StableSolver::stable ALIAS StableSolver_stable)

add_subdirectory(algorithms)

if(STABLESOLVER_BUILD_MAIN)
    add_executable(StableSolver_stable_main)
    target_sources(StableSolver_stable_main PRIVATE
        main.cpp)
    target_link_libraries(StableSolver_stable_main PUBLIC
        StableSolver_stable_greedy
        StableSolver_stable_milp
        StableSolver_stable_local_search
        StableSolver_stable_local_search_row_weighting
        StableSolver_stable_large_neighborhood_search
        Boost::program_options)
    set_target_properties(StableSolver_stable_main PROPERTIES OUTPUT_NAME "stablesolver_stable")
    install(TARGETS StableSolver_stable_main)

    add_executable(StableSolver_stable_mps_writer_main)
    target_sources(StableSolver_stable_mps_writer_main PRIVATE
        mps_writer_main.cpp)
    target_link_libraries(StableSolver_stable_mps_writer_main PUBLIC
        StableSolver_stable_milp
        Boost::program_options
        Boost::filesystem)
    set_target_properties(StableSolver_stable_mps_writer_main PROPERTIES OUTPUT_NAME "stablesolver_stable_mps_writer")
    install(TARGETS StableSolver_stable_mps_writer_main)
endif()
