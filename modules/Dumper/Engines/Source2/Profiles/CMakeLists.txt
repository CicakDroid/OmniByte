# Source2/Profiles — placeholder.
cmake_minimum_required(VERSION 3.22.1)

project(source2_profiles LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(source2_profiles STATIC source2_profiles.cpp)
