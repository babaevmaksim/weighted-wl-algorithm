cmake_minimum_required(VERSION 3.14)
project(weighted_wl_algorithm
    VERSION 1.0.0
    DESCRIPTION "Weighted Weisfeiler-Lehman (1-WL) Algorithm for Directed Attributed Graphs"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler optimization and warnings
if(MSVC)
    add_compile_options(/W4 /O2 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -O3)
endif()

# Core static library
add_library(weighted_wl STATIC
    src/graph.cpp
    src/wl_solver.cpp
    src/tudataset_reader.cpp
)

target_include_directories(weighted_wl PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Standalone CLI Executable
add_executable(wl_cli app/main.cpp)
target_link_libraries(wl_cli PRIVATE weighted_wl)

# Test Suite Executable
add_executable(wl_tests tests/test_wl.cpp)
target_link_libraries(wl_tests PRIVATE weighted_wl)

# Benchmark Suite Executable
add_executable(wl_benchmark benchmarks/benchmark.cpp)
target_link_libraries(wl_benchmark PRIVATE weighted_wl)

# Enable CTest integration
enable_testing()
add_test(NAME WeightedWL_UnitTests COMMAND wl_tests)
set_tests_properties(WeightedWL_UnitTests PROPERTIES WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

# Installation targets
include(GNUInstallDirs)
install(TARGETS weighted_wl wl_cli
    EXPORT WeightedWLTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
