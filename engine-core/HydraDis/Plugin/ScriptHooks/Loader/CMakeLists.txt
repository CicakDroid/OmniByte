# ScriptHooks/Loader — read & validate scripts before execution.
cmake_minimum_required(VERSION 3.22.1)

project(plugin_scripthooks_loader LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(plugin_scripthooks_loader STATIC plugin_scripthooks_loader.cpp)
target_include_directories(plugin_scripthooks_loader PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
)
