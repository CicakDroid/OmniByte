# HydraDis — static analysis engine (disassembly / decompilation / parsing / plugins).
cmake_minimum_required(VERSION 3.22.1)

project(hydradis LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(Disassembler)
add_subdirectory(Decompiler)
add_subdirectory(Parser)
add_subdirectory(Plugin)
add_subdirectory(Factory)
add_subdirectory(Orchestrator)

# Link shared metadata interface
target_link_libraries(hydradis PUBLIC shared)
