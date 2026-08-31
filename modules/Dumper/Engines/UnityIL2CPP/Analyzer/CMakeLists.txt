# UnityIL2CPP/Analyzer — placeholder.
cmake_minimum_required(VERSION 3.22.1)

project(unityil2cpp_analyzer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(unityil2cpp_analyzer STATIC unityil2cpp_analyzer.cpp)
