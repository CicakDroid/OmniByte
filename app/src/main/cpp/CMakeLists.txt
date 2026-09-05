# OmniByte native aggregator.
#
# This is the single CMake entry point referenced by app/build.gradle.kts
# via externalNativeBuild.cmake.path. It pulls in the three native roots
# (engine-core, runtime, modules/Dumper) and links them into one shared
# library `omnibyte` so the app produces a loadable libomnibyte.so.
cmake_minimum_required(VERSION 3.22.1)

project(OmniByteNative LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(ANDROID_STL c++_shared)

# This aggregator lives at <root>/app/src/main/cpp/, so the project root is
# four levels up (cpp -> main -> src -> app -> <root>).
set(OMNIBYTE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../../")

# ---------------------------------------------------------------------------
# Native roots (each owns its own CMakeLists.txt)
# ---------------------------------------------------------------------------
add_subdirectory(${OMNIBYTE_ROOT}/engine-core engine-core)
add_subdirectory(${OMNIBYTE_ROOT}/runtime runtime)
add_subdirectory(${OMNIBYTE_ROOT}/modules/Dumper dumper)

# ---------------------------------------------------------------------------
# Aggregated shared library
# ---------------------------------------------------------------------------
add_library(omnibyte SHARED placeholder.cpp)

target_include_directories(omnibyte PRIVATE
    ${OMNIBYTE_ROOT}/engine-core
    ${OMNIBYTE_ROOT}/runtime
    ${OMNIBYTE_ROOT}/modules/Dumper
)

# The three native roots currently compile to static placeholder libraries.
# Real backend adapters (capstone/rizin/lief/z3/cvc5/triton/xdl) are linked in
# later once each adapter CMakeLists has its find_package/ExternalProject_Add
# commented-out sections enabled.
target_link_libraries(omnibyte
    PRIVATE
        engine_core
        runtime
        dumper
        shared
)
