# Enhanced/CFG plugin — placeholder.
cmake_minimum_required(VERSION 3.22.1)

project(enhanced_cfg LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(enhanced_cfg STATIC enhanced_cfg.cpp)
target_include_directories(enhanced_cfg PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
)
