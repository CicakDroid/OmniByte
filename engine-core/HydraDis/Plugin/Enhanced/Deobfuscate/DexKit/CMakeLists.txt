# DexKit — DEX deobfuscation adapter.
cmake_minimum_required(VERSION 3.22.1)

project(dexkit_adapter LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(dexkit_adapter STATIC DexKitAdapter.cpp)
