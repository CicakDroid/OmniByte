# UNIGINE/Analyzer — placeholder.
cmake_minimum_required(VERSION 3.22.1)

project(unigine_analyzer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(unigine_analyzer STATIC unigine_analyzer.cpp)
