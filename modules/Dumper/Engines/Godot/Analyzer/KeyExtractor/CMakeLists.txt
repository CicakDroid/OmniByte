# KeyDot — Godot key extractor.
cmake_minimum_required(VERSION 3.22.1)

project(key_extractor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(key_extractor STATIC KeyExtractor.cpp)
