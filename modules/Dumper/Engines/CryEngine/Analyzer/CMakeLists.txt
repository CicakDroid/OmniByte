# CryEngine/Analyzer — placeholder.
cmake_minimum_required(VERSION 3.22.1)

project(cryengine_analyzer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(cryengine_analyzer STATIC cryengine_analyzer.cpp)
