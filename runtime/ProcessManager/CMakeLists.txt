# ProcessManager — process attach/detach, /proc parsing.
cmake_minimum_required(VERSION 3.22.1)

project(process_manager LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(process_manager STATIC process_manager.cpp)

target_include_directories(process_manager PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_link_libraries(process_manager PRIVATE common)
