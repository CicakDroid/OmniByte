# modules/Dumper — game-engine asset dumper.
cmake_minimum_required(VERSION 3.22.1)

project(dumper LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Base placeholder target for the dumper module.
add_library(dumper STATIC dumper.cpp)

add_subdirectory(DumperCore)
add_subdirectory(Engines)
add_subdirectory(Export)

# ---------------------------------------------------------------------------
# Real wiring (uncomment as sub-libraries become available):
#
# target_link_libraries(dumper PUBLIC
#     dumper_core
#     dumper_engines
#     dumper_export
# )
# ---------------------------------------------------------------------------
