# Source/Profiles — placeholder.
cmake_minimum_required(VERSION 3.22.1)

project(source_profiles LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(source_profiles STATIC source_profiles.cpp)
