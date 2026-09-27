# Disassembler — consumes disassembly backend adapters.
cmake_minimum_required(VERSION 3.22.1)

project(hydradis_disassembler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(backends)

# Factory dispatch (createDisassembler) — consumers link this target.
add_library(hydradis_disassembler STATIC
    HydraDisassembler.cpp
)

target_include_directories(hydradis_disassembler
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(hydradis_disassembler
    PUBLIC disassembler_capstone
    PUBLIC disassembler_protobuf
    PUBLIC disassembler_wasm
)
