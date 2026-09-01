# KittyMemory — Memory patching engine.
cmake_minimum_required(VERSION 3.22.1)

project(kitty_patch LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(kitty_patch STATIC KittyPatch.cpp)
