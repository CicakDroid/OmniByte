# engine-core — static analysis library (HydraDis).
cmake_minimum_required(VERSION 3.22.1)

project(engine_core LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Base placeholder target for the static-analysis engine core.
# engine_core.cpp was renamed to Hydra.cpp in the engine-core → Hydra refactor.
add_library(engine_core STATIC Hydra.cpp)

# HydraDis renamed to Hydra2D
add_subdirectory(Hydra2D)
add_subdirectory(Shared)

# Link shared to engine_core
target_link_libraries(engine_core PUBLIC shared)

# ---------------------------------------------------------------------------
# Real backend wiring (uncomment as each backend becomes available):
#
# target_link_libraries(engine_core PUBLIC
#     disassembler_capstone      # -> HydraDis/Disassembler/backends/capstone-adapter
#     decompiler_rizin_native    # -> HydraDis/Decompiler/backends/rizin-native
#     decompiler_rz_ghidra       # -> HydraDis/Decompiler/backends/rz-ghidra-adapter
#     parser_lief                # -> HydraDis/Parser/backends/lief-adapter
# )
# ---------------------------------------------------------------------------
