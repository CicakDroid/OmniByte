# Decompiler/backends/rz-ghidra-adapter — placeholder adapter for: rz-ghidra
#
# Stage 1 scaffolding only. This target compiles an empty stub so the build
# graph is complete. Real backend wiring lives BELOW the marker and is
# intentionally commented out — uncomment when the library is available.
cmake_minimum_required(VERSION 3.22.1)

project(decompiler_rz_ghidra LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(decompiler_rz_ghidra STATIC decompiler_rz_ghidra.cpp)

# ===== PLACEHOLDER — real backend wiring (uncomment when rz-ghidra is available) =====
# find_package(PkgConfig REQUIRED)
# pkg_check_modules(decompiler_rz_ghidra_PC REQUIRED rz-ghidra)
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendored  decompiler_rz_ghidra-vendored)
# ExternalProject_Add(decompiler_rz_ghidra_ep ...)
# target_include_directories(decompiler_rz_ghidra PUBLIC ${decompiler_rz_ghidra_PC_INCLUDE_DIRS})
# target_link_libraries(decompiler_rz_ghidra PUBLIC ${decompiler_rz_ghidra_PC_LIBRARIES})
