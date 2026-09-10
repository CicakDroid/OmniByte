# Dumper1 subdirectory CMakeLists
cmake_minimum_required(VERSION 3.22.1)

project(dumper1_resolver LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(dumper1_resolver INTERFACE)
target_include_directories(dumper1_resolver INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
