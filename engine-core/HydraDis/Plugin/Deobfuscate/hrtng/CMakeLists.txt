# hrtng — Extractable crypto/deob modules.
cmake_minimum_required(VERSION 3.22.1)

project(hrtng_deob LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(hrtng_deob STATIC HrtngDeob.cpp)
