cmake_minimum_required(VERSION 3.22.1)

project(parser_lief LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    lief
    GIT_REPOSITORY https://github.com/lief-project/LIEF.git
    GIT_TAG        1.0.0
    GIT_SHALLOW    TRUE
)

set(LIEF_PYTHON_API OFF CACHE BOOL "" FORCE)
set(LIEF_DOC OFF CACHE BOOL "" FORCE)
set(LIEF_TESTS OFF CACHE BOOL "" FORCE)
set(LIEF_EXAMPLES OFF CACHE BOOL "" FORCE)
set(LIEF_PE OFF CACHE BOOL "" FORCE)
set(LIEF_MACHO OFF CACHE BOOL "" FORCE)
set(LIEF_ELF ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(lief)

add_library(parser_lief STATIC parser_lief.cpp)

target_link_libraries(parser_lief PUBLIC LIEF::LIEF)

target_include_directories(parser_lief PUBLIC
    ${lief_SOURCE_DIR}/include
    ${lief_BINARY_DIR}/include
)
