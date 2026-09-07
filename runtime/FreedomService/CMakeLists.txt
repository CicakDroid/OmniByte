# FreedomService — hybrid root backend orchestrator.
cmake_minimum_required(VERSION 3.22.1)

project(freedom_service LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(freedom_service STATIC
    FreedomService.cpp
    backends/KernelSUAdapter.cpp
    backends/SukiSUUltraAdapter.cpp
    backends/SuiAdapter.cpp
    backends/RootThreadAdapter.cpp
)

target_include_directories(freedom_service PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_link_libraries(freedom_service PRIVATE common)
