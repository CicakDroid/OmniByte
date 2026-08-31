# Parser/backends/lief-adapter — placeholder adapter for: lief
#
# Stage 1 scaffolding only. This target compiles an empty stub so the build
# graph is complete. Real backend wiring lives BELOW the marker and is
# intentionally commented out — uncomment when the library is available.
cmake_minimum_required(VERSION 3.22.1)

project(parser_lief LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(parser_lief STATIC parser_lief.cpp)

# ===== PLACEHOLDER — real backend wiring (uncomment when lief is available) =====
# find_package(PkgConfig REQUIRED)
# pkg_check_modules(parser_lief_PC REQUIRED lief)
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendored  parser_lief-vendored)
# ExternalProject_Add(parser_lief_ep ...)
# target_include_directories(parser_lief PUBLIC ${parser_lief_PC_INCLUDE_DIRS})
# target_link_libraries(parser_lief PUBLIC ${parser_lief_PC_LIBRARIES})
