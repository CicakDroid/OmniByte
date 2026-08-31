# UNIGINE/Profiles — placeholder.
cmake_minimum_required(VERSION 3.22.1)

project(unigine_profiles LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(unigine_profiles STATIC unigine_profiles.cpp)
