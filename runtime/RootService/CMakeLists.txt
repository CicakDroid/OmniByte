# RootThread — Root service IPC wrapper.
cmake_minimum_required(VERSION 3.22.1)

project(root_thread LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(root_thread STATIC RootThread.cpp)

target_link_libraries(root_thread PRIVATE pthread)
