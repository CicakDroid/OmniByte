# DumperCore/SharedUtils — placeholder.
cmake_minimum_required(VERSION 3.22.1)

project(dumper_core_sharedutils LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(dumper_core_sharedutils STATIC dumper_core_sharedutils.cpp)
