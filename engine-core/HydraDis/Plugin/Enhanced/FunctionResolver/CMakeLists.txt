# Enhanced/FunctionResolver plugin — placeholder.
cmake_minimum_required(VERSION 3.22.1)

project(enhanced_functionresolver LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(enhanced_functionresolver STATIC enhanced_functionresolver.cpp)
target_include_directories(enhanced_functionresolver PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
)
