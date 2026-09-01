# Orchestrator — Pipeline orchestrator untuk HydraDis.
cmake_minimum_required(VERSION 3.22.1)

project(hydradis_orchestrator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(hydradis_orchestrator STATIC
    HydraDisOrchestrator.cpp
)

target_include_directories(hydradis_orchestrator
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(hydradis_orchestrator
    PUBLIC hydradis_factory
)
