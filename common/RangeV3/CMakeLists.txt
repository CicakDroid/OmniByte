# common/RangeV3 — range-v3 (BSL-1.0) range adaptors.
cmake_minimum_required(VERSION 3.22.1)
project(omnibyte_rangev3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(RANGES_CXX_STD 17 CACHE STRING "" FORCE)
# range-v3 0.12 gate names: RANGE_V3_DOCS/TESTS/EXAMPLES/PERF (CMAKE_DEPENDENT_OPTION);
# FORCE so FetchContent subproject never re-enables doc HTML / gtest pulls.
set(RANGE_V3_DOCS OFF CACHE BOOL "" FORCE)
set(RANGE_V3_TESTS OFF CACHE BOOL "" FORCE)
set(RANGE_V3_EXAMPLES OFF CACHE BOOL "" FORCE)
set(RANGE_V3_PERF OFF CACHE BOOL "" FORCE)
set(RANGES_BUILD_CALENDAR_EXAMPLE OFF CACHE BOOL "" FORCE)
set(RANGES_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(RANGES_BUILD_PERF_TESTS OFF CACHE BOOL "" FORCE)
set(RANGES_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(RANGES_DOXYGEN OFF CACHE BOOL "" FORCE)

include(FetchContent)
cmake_policy(SET CMP0135 NEW)
FetchContent_Declare(
    range_v3
    GIT_REPOSITORY https://github.com/ericniebler/range-v3.git
    GIT_TAG        0.12.0
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(range_v3)

add_library(omnibyte_rangev3 STATIC RangeV3.cpp)

target_include_directories(omnibyte_rangev3 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
)
# range-v3 creates only the range-v3::range-v3 alias (range_v3-src/CMakeLists.txt).
target_link_libraries(omnibyte_rangev3 PUBLIC range-v3::range-v3)
