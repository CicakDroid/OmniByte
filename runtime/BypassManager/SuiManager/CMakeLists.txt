# Sui SuperUser manager wrapper.
cmake_minimum_required(VERSION 3.22.1)

project(sui_manager LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(sui_manager STATIC SuiManager.cpp)
