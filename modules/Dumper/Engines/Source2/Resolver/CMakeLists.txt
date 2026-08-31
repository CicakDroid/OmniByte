# Source2/Resolver — placeholder.
cmake_minimum_required(VERSION 3.22.1)

project(source2_resolver LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(source2_resolver STATIC source2_resolver.cpp)
