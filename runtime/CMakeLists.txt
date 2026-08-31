# runtime — live device concerns (attach, memory I/O, symbol resolution, roots).
cmake_minimum_required(VERSION 3.22.1)

project(runtime LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Base placeholder target for the live runtime library.
add_library(runtime STATIC runtime.cpp)

add_subdirectory(Bridges)
add_subdirectory(ProcessManager)
add_subdirectory(MemoryIO)
add_subdirectory(SymbolResolver)

# ---------------------------------------------------------------------------
# Real wiring (uncomment as sub-libraries become available):
#
# target_link_libraries(runtime PUBLIC
#     bridge_java_native
#     bridge_root_service
#     bridge_module
#     process_manager
#     memory_io
#     symbols_xdl
# )
# ---------------------------------------------------------------------------
