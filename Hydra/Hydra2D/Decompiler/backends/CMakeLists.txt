# Decompiler backends — each adapter is a self-contained placeholder target.
cmake_minimum_required(VERSION 3.22.1)

project(hydradis_decompiler_backends LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(rizin-native)
# rz-ghidra needs an installed Rizin CMake package; in-tree rizin is Meson-only
# (no CMakeLists), so FetchContent_MakeAvailable(rz_ghidra) always fails at
# find_package(Rizin). Not linked into engine_core yet — re-enable after a
# prebuilt Rizin is installed (toolchain/rizin-android).
# add_subdirectory(rz-ghidra-adapter)
