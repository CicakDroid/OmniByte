# FindCrypt3 — Crypto constant finder.
cmake_minimum_required(VERSION 3.22.1)

project(findcrypt3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(findcrypt3 STATIC FindCrypt3.cpp)
