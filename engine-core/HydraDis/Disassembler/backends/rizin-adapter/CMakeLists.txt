# Disassembler/backends/rizin-adapter — placeholder adapter for: rizin
#
# Stage 1 scaffolding only. This target compiles an empty stub so the build
# graph is complete. Real backend wiring lives BELOW the marker and is
# intentionally commented out — uncomment when the library is available.
cmake_minimum_required(VERSION 3.22.1)

project(disassembler_rizin LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(disassembler_rizin STATIC disassembler_rizin.cpp)

# ===== PLACEHOLDER — real backend wiring (uncomment when rizin is available) =====
# find_package(PkgConfig REQUIRED)
# pkg_check_modules(disassembler_rizin_PC REQUIRED rizin)
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendored  disassembler_rizin-vendored)
# ExternalProject_Add(disassembler_rizin_ep ...)
# target_include_directories(disassembler_rizin PUBLIC ${disassembler_rizin_PC_INCLUDE_DIRS})
# target_link_libraries(disassembler_rizin PUBLIC ${disassembler_rizin_PC_LIBRARIES})
