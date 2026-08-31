# Bridges/JavaNativeBridge — placeholder.
cmake_minimum_required(VERSION 3.22.1)

project(bridge_java_native LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(bridge_java_native STATIC bridge_java_native.cpp)
