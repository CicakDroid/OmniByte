cmake_minimum_required(VERSION 3.22.1)

project(decompiler_rz_ghidra LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# Build rizin first (rz-ghidra depends on it)
FetchContent_Declare(
    rizin
    GIT_REPOSITORY https://github.com/rizinorg/rizin.git
    GIT_TAG        v0.9.1
    GIT_SHALLOW    TRUE
)

# rz-ghidra: Ghidra decompiler as rizin plugin (C++ only, no Java/Ghidra)
FetchContent_Declare(
    rz_ghidra
    GIT_REPOSITORY https://github.com/rizinorg/rz-ghidra.git
    GIT_TAG        v0.9.0
    GIT_SHALLOW    TRUE
)

set(RIZIN_BUILD_STATIC ON CACHE BOOL "" FORCE)
set(RIZIN_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(BUILD_TESTS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(rizin)

# rz-ghidra links against rizin; build after rizin is available
FetchContent_MakeAvailable(rz_ghidra)

add_library(decompiler_rz_ghidra STATIC decompiler_rz_ghidra.cpp)

target_link_libraries(decompiler_rz_ghidra PUBLIC
    rizin
    rz_ghidra
)

target_include_directories(decompiler_rz_ghidra PUBLIC
    ${rizin_SOURCE_DIR}/librz/include
    ${rz_ghidra_SOURCE_DIR}/src
)
