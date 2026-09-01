cmake_minimum_required(VERSION 3.22.1)

project(symbolicexec_triton LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    triton
    GIT_REPOSITORY https://github.com/JonathanSalwan/Triton.git
    GIT_TAG        v0.9
    GIT_SHALLOW    TRUE
)

set(TRITON_BUILD_PYTHON_BINDINGS OFF CACHE BOOL "" FORCE)
set(TRITON_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(TRITON_BUILD_SHARED OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(triton)

add_library(symbolicexec_triton STATIC symbolicexec_triton.cpp)

target_link_libraries(symbolicexec_triton PUBLIC triton)

target_include_directories(symbolicexec_triton PUBLIC
    ${triton_SOURCE_DIR}/src/libtriton/includes
    ${triton_BINARY_DIR}/src/libtriton
)
