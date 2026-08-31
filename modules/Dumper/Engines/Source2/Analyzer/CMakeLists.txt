# Source2/Analyzer — placeholder.
cmake_minimum_required(VERSION 3.22.1)

project(source2_analyzer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(source2_analyzer STATIC source2_analyzer.cpp)
