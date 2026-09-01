# bhook — PLT hook engine wrapper.
cmake_minimum_required(VERSION 3.22.1)

project(plthook LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(plthook STATIC PLTHook.cpp)

target_link_libraries(plthook PRIVATE dl)
