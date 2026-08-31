# Decompiler/backends/rizin-native — placeholder adapter for: rizin
#
# Stage 1 scaffolding only. This target compiles an empty stub so the build
# graph is complete. Real backend wiring lives BELOW the marker and is
# intentionally commented out — uncomment when the library is available.
cmake_minimum_required(VERSION 3.22.1)

project(decompiler_rizin_native LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(decompiler_rizin_native STATIC decompiler_rizin_native.cpp)

# ===== PLACEHOLDER — real backend wiring (uncomment when rizin is available) =====
# find_package(PkgConfig REQUIRED)
# pkg_check_modules(decompiler_rizin_native_PC REQUIRED rizin)
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendored  decompiler_rizin_native-vendored)
# ExternalProject_Add(decompiler_rizin_native_ep ...)
# target_include_directories(decompiler_rizin_native PUBLIC ${decompiler_rizin_native_PC_INCLUDE_DIRS})
# target_link_libraries(decompiler_rizin_native PUBLIC ${decompiler_rizin_native_PC_LIBRARIES})
