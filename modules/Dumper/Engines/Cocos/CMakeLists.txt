# Dumper Engine: Cocos2d — Analyzer + Resolver + Profiles.
cmake_minimum_required(VERSION 3.22.1)

project(engine_cocos2d LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(Analyzer)
add_subdirectory(Resolver)
add_subdirectory(Profiles)
