# Enhanced/Emulation/Qemu — Anti-detection wrapper.
# Berdasarkan zhaodice/qemu-anti-detection (patch untuk QEMU 6.2.0-10.2.2).
cmake_minimum_required(VERSION 3.22.1)

project(emulation_qemu LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(emulation_qemu STATIC
    qemu_antidetect.cpp
)

target_include_directories(emulation_qemu PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
)
