# Enhanced/Emulation — CPU emulation via Unicorn Engine + anti-detection.
cmake_minimum_required(VERSION 3.22.1)

project(enhanced_emulation LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(Unicorn)
add_subdirectory(Qemu)

add_library(enhanced_emulation STATIC enhanced_emulation.cpp)
target_include_directories(enhanced_emulation PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
)
target_link_libraries(enhanced_emulation PRIVATE
    emulation_unicorn
    emulation_qemu
)
