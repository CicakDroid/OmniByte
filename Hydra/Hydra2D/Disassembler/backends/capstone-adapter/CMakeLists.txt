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

set(CAPSTONE_BUILD_STATIC ON CACHE BOOL "" FORCE)
set(CAPSTONE_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(CAPSTONE_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(CAPSTONE_BUILD_CSTOOL OFF CACHE BOOL "" FORCE)
set(CAPSTONE_ARM_SUPPORT ON CACHE BOOL "" FORCE)
set(CAPSTONE_ARM64_SUPPORT ON CACHE BOOL "" FORCE)
set(CAPSTONE_X86_SUPPORT ON CACHE BOOL "" FORCE)
set(CAPSTONE_MIPS_SUPPORT ON CACHE BOOL "" FORCE)
set(CAPSTONE_PPC_SUPPORT ON CACHE BOOL "" FORCE)
set(CAPSTONE_SPARC_SUPPORT ON CACHE BOOL "" FORCE)
set(CAPSTONE_SYSZ_SUPPORT ON CACHE BOOL "" FORCE)
set(CAPSTONE_XCORE_SUPPORT ON CACHE BOOL "" FORCE)
set(CAPSTONE_M68K_SUPPORT ON CACHE BOOL "" FORCE)
set(CAPSTONE_TMS320C64X_SUPPORT ON CACHE BOOL "" FORCE)
set(CAPSTONE_M680X_SUPPORT ON CACHE BOOL "" FORCE)
set(CAPSTONE_EVM_SUPPORT ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(capstone)

add_library(disassembler_capstone STATIC disassembler_capstone.cpp)

target_link_libraries(disassembler_capstone PUBLIC capstone-static)

target_include_directories(disassembler_capstone PUBLIC
    ${capstone_SOURCE_DIR}/include
    ${capstone_BINARY_DIR}/include
)
