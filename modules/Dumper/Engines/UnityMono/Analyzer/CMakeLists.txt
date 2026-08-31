# UnityMono/Analyzer — static metadata parser.
cmake_minimum_required(VERSION 3.22.1)

project(unitymono_analyzer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(unitymono_analyzer STATIC unitymono_analyzer.cpp)
target_include_directories(unitymono_analyzer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
