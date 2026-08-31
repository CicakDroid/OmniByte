# DumperCore/Detector — placeholder.
cmake_minimum_required(VERSION 3.22.1)

project(dumper_core_detector LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(dumper_core_detector STATIC dumper_core_detector.cpp)
