# Parser backends — each adapter is a self-contained placeholder target.
cmake_minimum_required(VERSION 3.22.1)

project(hydradis_parser_backends LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(lief-adapter)
