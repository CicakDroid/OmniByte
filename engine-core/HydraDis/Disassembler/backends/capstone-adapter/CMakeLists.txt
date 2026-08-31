# Disassembler/backends/capstone-adapter — placeholder adapter for: capstone
#
# Stage 1 scaffolding only. This target compiles an empty stub so the build
# graph is complete. Real backend wiring lives BELOW the marker and is
# intentionally commented out — uncomment when the library is available.
cmake_minimum_required(VERSION 3.22.1)

project(disassembler_capstone LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(disassembler_capstone STATIC disassembler_capstone.cpp)

# ===== PLACEHOLDER — real backend wiring (uncomment when capstone is available) =====
# find_package(PkgConfig REQUIRED)
# pkg_check_modules(disassembler_capstone_PC REQUIRED capstone)
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendored  disassembler_capstone-vendored)
# ExternalProject_Add(disassembler_capstone_ep ...)
# target_include_directories(disassembler_capstone PUBLIC ${disassembler_capstone_PC_INCLUDE_DIRS})
# target_link_libraries(disassembler_capstone PUBLIC ${disassembler_capstone_PC_LIBRARIES})
