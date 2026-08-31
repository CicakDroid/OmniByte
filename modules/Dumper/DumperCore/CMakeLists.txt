# DumperCore — orchestrator (registry, detection, shared utils, normalization).
cmake_minimum_required(VERSION 3.22.1)

project(dumper_core LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(EngineRegistry)
add_subdirectory(Detector)
add_subdirectory(SharedUtils)
add_subdirectory(ResultNormalizer)
