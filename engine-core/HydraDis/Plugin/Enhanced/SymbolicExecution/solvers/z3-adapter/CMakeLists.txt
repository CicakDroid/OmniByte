cmake_minimum_required(VERSION 3.22.1)

project(solvers_z3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    z3
    GIT_REPOSITORY https://github.com/Z3Prover/z3.git
    GIT_TAG        z3-4.13.0
    GIT_SHALLOW    TRUE
)

set(Z3_BUILD_PYTHON_BINDINGS OFF CACHE BOOL "" FORCE)
set(Z3_BUILD_JAVA_BINDINGS OFF CACHE BOOL "" FORCE)
set(Z3_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(Z3_BUILD_DOCUMENTATION OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(z3)

add_library(solvers_z3 STATIC solvers_z3.cpp)

target_link_libraries(solvers_z3 PUBLIC libz3)

target_include_directories(solvers_z3 PUBLIC
    ${z3_SOURCE_DIR}/src/api
    ${z3_BINARY_DIR}
)
