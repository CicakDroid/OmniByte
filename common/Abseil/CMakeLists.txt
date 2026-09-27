# common/Abseil — abseil-cpp (Apache-2.0) fast containers + string helpers.
cmake_minimum_required(VERSION 3.22.1)
project(omnibyte_abseil LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(ABSL_PROPAGATE_CXX_STD ON)
set(ABSL_ENABLE_INSTALL OFF)
set(ABSL_BUILD_TESTING OFF)
set(ABSL_BUILD_EXAMPLES OFF)

include(FetchContent)
cmake_policy(SET CMP0135 NEW)
FetchContent_Declare(
    abseil_cpp
    GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git
    GIT_TAG        20250127.1
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(abseil_cpp)

add_library(omnibyte_abseil STATIC Abseil.cpp)

target_include_directories(omnibyte_abseil PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
)
target_link_libraries(omnibyte_abseil
    PUBLIC
        absl::flat_hash_map
        absl::flat_hash_set
        # str_join/str_split ship inside absl::strings — no such standalone targets exist.
        absl::strings
)
