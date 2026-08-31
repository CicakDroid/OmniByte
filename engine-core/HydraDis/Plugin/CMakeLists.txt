# HydraDis Plugins — all implement the IPlugin contract (toggleable).
cmake_minimum_required(VERSION 3.22.1)

project(hydradis_plugin LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(Deobfuscate)
add_subdirectory(ScriptHooks)
add_subdirectory(Enhanced)
