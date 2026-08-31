# Parser — consumes binary-format parsing backend adapters (ELF/PE/Mach-O).
cmake_minimum_required(VERSION 3.22.1)

project(hydradis_parser LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(backends)
