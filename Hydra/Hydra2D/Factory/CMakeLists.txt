# Factory — Backend factory classes untuk HydraDis.
cmake_minimum_required(VERSION 3.22.1)

project(hydradis_factory LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Factory sources
add_library(hydradis_factory STATIC
    DisassemblerFactory.cpp
    ParserFactory.cpp
    DecompilerFactory.cpp
    SolverFactory.cpp
)

target_include_directories(hydradis_factory
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..
)

# Link backend interfaces (header-only, no library deps yet)
# Backend .cpp files are compiled in their own subdirectories via FetchContent.
# Factory only forward-declares their free functions.
target_link_libraries(hydradis_factory
    PUBLIC hydradis_disassembler_backends
    PUBLIC hydradis_parser_backends
    PUBLIC hydradis_decompiler_backends
)
