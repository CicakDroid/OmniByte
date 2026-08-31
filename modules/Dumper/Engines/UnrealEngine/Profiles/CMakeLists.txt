# UnrealEngine/Profiles — placeholder.
cmake_minimum_required(VERSION 3.22.1)

project(unrealengine_profiles LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(unrealengine_profiles STATIC unrealengine_profiles.cpp)
