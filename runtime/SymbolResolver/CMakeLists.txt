# runtime/SymbolResolver — dynamic symbol resolution across Android versions.
cmake_minimum_required(VERSION 3.22.1)

project(runtime_symbolresolver LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(xdl-adapter)
