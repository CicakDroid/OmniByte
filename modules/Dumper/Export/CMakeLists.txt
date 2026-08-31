# modules/Dumper/Export — export pipeline (core + format writers).
cmake_minimum_required(VERSION 3.22.1)

project(dumper_export LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(ExportCore)
add_subdirectory(Writers)
