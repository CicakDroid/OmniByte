# runtime — live device concerns (attach, memory I/O, symbol resolution, roots).
cmake_minimum_required(VERSION 3.22.1)

project(runtime LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(runtime STATIC Runtime.cpp)

add_subdirectory(ProcessManager)
add_subdirectory(MemoryIO)
add_subdirectory(SymbolResolver)
add_subdirectory(FreedomService)
add_subdirectory(ZigZag)
add_subdirectory(ZigZagManager)
add_subdirectory(HPT)
add_subdirectory(Bridges)

target_include_directories(runtime PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_link_libraries(runtime PUBLIC
    process_manager
    memory_io
    symbols_xdl
    freedom_service
    zigzag
    zigzag_manager
    hpt
    bridge_freedom_service
    bridge_module
    dumper
    common
)
