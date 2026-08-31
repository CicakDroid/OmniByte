# Source/Resolver — placeholder.
cmake_minimum_required(VERSION 3.22.1)

project(source_resolver LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(source_resolver STATIC source_resolver.cpp)
