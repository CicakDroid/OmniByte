# Diamorphine LKM rootkit reference wrapper.
cmake_minimum_required(VERSION 3.22.1)

project(diamorphine_reference LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(diamorphine_reference STATIC Diamorphine.cpp)
