# stealth-poc — Traceless KPM hook.
cmake_minimum_required(VERSION 3.22.1)

project(traceless_hook LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(traceless_hook STATIC TracelessHook.cpp)
