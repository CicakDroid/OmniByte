# ExportCore — exporter contracts, registry, and output splitting.
cmake_minimum_required(VERSION 3.22.1)

project(dumper_export_core LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(IExporter)
add_subdirectory(ExportRegistry)
add_subdirectory(SectionSplitter)
