# Dumper Engine: UnityIL2CPP — Analyzer + Resolver + Profiles.
cmake_minimum_required(VERSION 3.22.1)

project(engine_unityil2cpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(Analyzer)
add_subdirectory(Resolver)
add_subdirectory(Profiles)
