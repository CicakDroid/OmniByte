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
# Common libraries (header-only / static adapters)
# ---------------------------------------------------------------------------
# `common` and `log` are linked by runtime/HPT/TaskFlow but never defined as
# targets anywhere in-tree — provide INTERFACE shims before any add_subdirectory.
if(NOT TARGET common)
    add_library(common INTERFACE)
    target_include_directories(common INTERFACE
        "${OMNIBYTE_ROOT}"
        "${OMNIBYTE_ROOT}/common"
    )
endif()
if(NOT TARGET log)
    # Bare `-llog` avoids a CMake target named `log` colliding with NDK liblog.
    add_library(log INTERFACE)
    target_link_libraries(log INTERFACE "-llog")
endif()

add_subdirectory(${OMNIBYTE_ROOT}/common/TaskFlow          taskflow)
add_subdirectory(${OMNIBYTE_ROOT}/common/GMP               gmp_adapter)
add_subdirectory(${OMNIBYTE_ROOT}/common/Highway highway)
add_subdirectory(${OMNIBYTE_ROOT}/common/Boost boost_adapter)
add_subdirectory(${OMNIBYTE_ROOT}/common/Abseil abseil)
add_subdirectory(${OMNIBYTE_ROOT}/common/RangeV3 rangev3)
add_subdirectory(${OMNIBYTE_ROOT}/common/Deserialization-Serialization/Json json_adapter)
add_subdirectory(${OMNIBYTE_ROOT}/common/Eigen               eigen)

# config/*Config.h and Dumper.cpp include <nlohmann/json.hpp> directly.
target_link_libraries(common INTERFACE nlohmann_json::nlohmann_json)

# ---------------------------------------------------------------------------
# Native roots (each owns its own CMakeLists.txt)
# ---------------------------------------------------------------------------
# engine-core → Hydra, modules/Hooker → modules/HPT (renamed in git history).
add_subdirectory(${OMNIBYTE_ROOT}/Hydra engine-core)
add_subdirectory(${OMNIBYTE_ROOT}/runtime runtime)
add_subdirectory(${OMNIBYTE_ROOT}/modules/Dumper dumper)
add_subdirectory(${OMNIBYTE_ROOT}/modules/HPT hooker)

# ---------------------------------------------------------------------------
# Aggregated shared library
# ---------------------------------------------------------------------------
add_library(omnibyte SHARED placeholder.cpp)

target_include_directories(omnibyte PRIVATE
    ${OMNIBYTE_ROOT}/Hydra
    ${OMNIBYTE_ROOT}/runtime
    ${OMNIBYTE_ROOT}/modules/Dumper
    ${OMNIBYTE_ROOT}/modules/HPT
)

# The three native roots currently compile to static placeholder libraries.
# Real backend adapters (capstone/rizin/lief/z3/cvc5/triton/xdl) are linked in
# later once each adapter CMakeLists has its find_package/ExternalProject_Add
# commented-out sections enabled.
# `hpt` lives under modules/HPT; `shared` comes from Hydra/Shared/Metadata.
target_link_libraries(omnibyte
    PRIVATE
        engine_core
        runtime
        dumper
        hpt
        shared
)
