cmake_minimum_required(VERSION 3.22.1)

project(disassembler_capstone LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    capstone
    GIT_REPOSITORY https://github.com/capstone-engine/capstone.git
    GIT_TAG        5.0.9
    GIT_SHALLOW    TRUE
)

# capstone_static target hanya dibuat kalau ini ON; static only untuk Android.
set(BUILD_STATIC_LIBS   ON  CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS   OFF CACHE BOOL "" FORCE)
set(CAPSTONE_BUILD_TESTS  OFF CACHE BOOL "" FORCE)
set(CAPSTONE_BUILD_CSTOOL OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(capstone)

add_library(disassembler_capstone STATIC CapstoneAdapter.cpp)

target_link_libraries(disassembler_capstone PUBLIC capstone_static)

target_include_directories(disassembler_capstone PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
    ${capstone_SOURCE_DIR}/include
)
