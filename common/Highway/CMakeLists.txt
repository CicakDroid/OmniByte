# common/Highway — Google Highway (Apache-2.0) portable SIMD.
cmake_minimum_required(VERSION 3.22.1)
project(omnibyte_highway LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Force cache so Highway's `set(... CACHE BOOL ON)` default cannot re-enable
# tests; its googletest download configures with cmake_minimum_required < 3.5
# and hard-fails under CMake 4.x.
set(HWY_ENABLE_TESTS OFF CACHE BOOL "Enable HWY tests" FORCE)
set(HWY_ENABLE_EXAMPLES OFF CACHE BOOL "Enable HWY examples" FORCE)
set(HWY_ENABLE_CONTRIB OFF CACHE BOOL "Enable HWY contrib" FORCE)
set(HWY_ENABLE_MANUAL_TESTS OFF CACHE BOOL "Enable HWY manual tests" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "Build tests" FORCE)

include(FetchContent)
cmake_policy(SET CMP0135 NEW)
FetchContent_Declare(
    highway
    GIT_REPOSITORY https://github.com/google/highway.git
    GIT_TAG        1.2.0
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(highway)

add_library(omnibyte_highway STATIC Highway.cpp)

target_include_directories(omnibyte_highway PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
)
# highway 1.2.0 in-tree build defines plain `hwy`, not the find_package alias hwy::hwy.
target_link_libraries(omnibyte_highway PUBLIC hwy)
