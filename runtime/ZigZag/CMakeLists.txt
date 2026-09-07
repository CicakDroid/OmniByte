# ZigZag — stealth backend (Diamorphine + Bypasser adapters).
cmake_minimum_required(VERSION 3.22.1)

project(zigzag LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(zigzag STATIC
    ZigZag.cpp
    backends/DiamorphineAdapter.cpp
    backends/BypasserAdapter.cpp
)

target_include_directories(zigzag PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_link_libraries(zigzag PRIVATE common)
