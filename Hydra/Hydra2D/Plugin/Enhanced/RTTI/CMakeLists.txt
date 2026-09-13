# Enhanced/RTTI plugin — placeholder.
cmake_minimum_required(VERSION 3.22.1)

project(enhanced_rtti LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(enhanced_rtti STATIC enhanced_rtti.cpp)
target_include_directories(enhanced_rtti PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
)
