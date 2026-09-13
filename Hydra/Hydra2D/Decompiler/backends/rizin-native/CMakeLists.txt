cmake_minimum_required(VERSION 3.22.1)

project(decompiler_rizin_native LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(decompiler_rizin_native STATIC decompiler_rizin_native.cpp)

target_include_directories(decompiler_rizin_native PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
)

# Subprocess approach: calls `rizin -j` via popen().
# No library linkage required. Rizin binary must be available in PATH
# or at a configured path at runtime.
