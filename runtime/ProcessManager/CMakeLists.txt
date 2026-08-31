# ProcessManager — placeholder.
cmake_minimum_required(VERSION 3.22.1)

project(process_manager LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(process_manager STATIC process_manager.cpp)
