# UnityIL2CPP/Profiles — placeholder.
cmake_minimum_required(VERSION 3.22.1)

project(unityil2cpp_profiles LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(unityil2cpp_profiles STATIC unityil2cpp_profiles.cpp)
