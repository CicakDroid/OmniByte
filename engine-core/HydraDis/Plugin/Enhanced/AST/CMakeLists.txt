# Enhanced/AST plugin — placeholder.
cmake_minimum_required(VERSION 3.22.1)

project(enhanced_ast LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(enhanced_ast STATIC enhanced_ast.cpp)
