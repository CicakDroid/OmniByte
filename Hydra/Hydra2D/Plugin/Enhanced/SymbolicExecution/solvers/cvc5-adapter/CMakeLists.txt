cmake_minimum_required(VERSION 3.22.1)

project(solvers_cvc5 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    cvc5
    GIT_REPOSITORY https://github.com/cvc5/cvc5.git
    GIT_TAG        cvc5-1.3.4
    GIT_SHALLOW    TRUE
)

set(CVC5_BUILD_PYTHON_BINDINGS OFF CACHE BOOL "" FORCE)
set(CVC5_BUILD_JAVA_BINDINGS OFF CACHE BOOL "" FORCE)
set(CVC5_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(CVC5_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(CVC5_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(cvc5)

add_library(solvers_cvc5 STATIC solvers_cvc5.cpp)

target_link_libraries(solvers_cvc5 PUBLIC cvc5)

target_include_directories(solvers_cvc5 PUBLIC
    ${cvc5_SOURCE_DIR}/include
    ${cvc5_BINARY_DIR}
)
