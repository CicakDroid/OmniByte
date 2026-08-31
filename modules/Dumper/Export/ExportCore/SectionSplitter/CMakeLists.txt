# ExportCore/SectionSplitter — placeholder.
cmake_minimum_required(VERSION 3.22.1)

project(dumper_export_sectionsplitter LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(dumper_export_sectionsplitter STATIC dumper_export_sectionsplitter.cpp)
