# ScriptHooks/Runner — execute scripts in a sandbox separate from Loader.
cmake_minimum_required(VERSION 3.22.1)

project(plugin_scripthooks_runner LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(plugin_scripthooks_runner STATIC plugin_scripthooks_runner.cpp)
