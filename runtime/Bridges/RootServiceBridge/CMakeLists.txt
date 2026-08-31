# Bridges/RootServiceBridge — placeholder.
cmake_minimum_required(VERSION 3.22.1)

project(bridge_root_service LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(bridge_root_service STATIC bridge_root_service.cpp)
