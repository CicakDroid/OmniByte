# MemoryIO — placeholder.
cmake_minimum_required(VERSION 3.22.1)

project(memory_io LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(memory_io STATIC memory_io.cpp)
