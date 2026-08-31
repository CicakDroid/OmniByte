# GameMaker/Analyzer — placeholder.
cmake_minimum_required(VERSION 3.22.1)

project(gamemaker_analyzer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(gamemaker_analyzer STATIC gamemaker_analyzer.cpp)
