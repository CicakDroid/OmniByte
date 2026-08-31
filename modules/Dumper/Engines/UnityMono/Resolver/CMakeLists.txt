# UnityMono/Resolver — placeholder.
cmake_minimum_required(VERSION 3.22.1)

project(unitymono_resolver LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(unitymono_resolver STATIC unitymono_resolver.cpp)
