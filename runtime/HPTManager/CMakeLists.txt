# HPTManager — manages HPT at runtime level.
cmake_minimum_required(VERSION 3.22.1)

project(hpt_manager LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(hpt_manager STATIC HPTManager.cpp)

target_include_directories(hpt_manager PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_link_libraries(hpt_manager PRIVATE hpt dumper common log)
