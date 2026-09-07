# ZigZagManager — selects best stealth backend.
cmake_minimum_required(VERSION 3.22.1)

project(zigzag_manager LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(zigzag_manager STATIC ZigZagManager.cpp)

target_include_directories(zigzag_manager PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_link_libraries(zigzag_manager PRIVATE zigzag dumper common)
