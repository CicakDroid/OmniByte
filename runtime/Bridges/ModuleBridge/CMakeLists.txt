# Bridges/ModuleBridge — placeholder.
cmake_minimum_required(VERSION 3.22.1)

project(bridge_module LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(bridge_module STATIC bridge_module.cpp)
