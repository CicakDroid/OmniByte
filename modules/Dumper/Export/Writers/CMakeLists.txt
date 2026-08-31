# Export/Writers — one writer per output format.
cmake_minimum_required(VERSION 3.22.1)

project(dumper_export_writers LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(CSharpWriter)
add_subdirectory(HeaderWriter)
add_subdirectory(JsonWriter)
add_subdirectory(DummyDllWriter)
