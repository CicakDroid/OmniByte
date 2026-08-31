# Enhanced/Emulation plugin — placeholder.
cmake_minimum_required(VERSION 3.22.1)

project(enhanced_emulation LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(enhanced_emulation STATIC enhanced_emulation.cpp)
