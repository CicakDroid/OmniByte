# Dumper Engines — one dir per engine, identical pattern:
#   <Engine>/Analyzer   — engine variant parsing
#   <Engine>/Resolver   — structure/offset resolution
#   <Engine>/Profiles   — version profiles
cmake_minimum_required(VERSION 3.22.1)

project(dumper_engines LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(UnrealEngine)
add_subdirectory(UnityIL2CPP)
add_subdirectory(UnityMono)
add_subdirectory(Source2)
add_subdirectory(Godot)
add_subdirectory(GameMaker)
