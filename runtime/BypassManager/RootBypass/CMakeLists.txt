# Bypasser — Root detection bypass.
cmake_minimum_required(VERSION 3.22.1)

project(bypasser LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(bypasser STATIC Bypasser.cpp)
