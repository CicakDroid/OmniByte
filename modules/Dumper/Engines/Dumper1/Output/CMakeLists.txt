# Dumper1 Output — dump.cs / struct / static field export.
# Header-only; no library target needed.
cmake_minimum_required(VERSION 3.22.1)

project(dumper1_output LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Header-only — no add_library.  Consumers include via target_include_directories.
