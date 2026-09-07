# HPT — Hooking Platform Toolkit orchestrator.
cmake_minimum_required(VERSION 3.22.1)

project(hpt LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(hpt STATIC
    HPT.cpp
    backends/AlbatrossAdapter.cpp
    backends/BhookAdapter.cpp
    backends/InlinehookAdapter.cpp
    backends/VectorAdapter.cpp
    backends/KittyMemoryAdapter.cpp
    backends/KittyMemoryExAdapter.cpp
)

target_include_directories(hpt PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_link_libraries(hpt PRIVATE dumper common)
