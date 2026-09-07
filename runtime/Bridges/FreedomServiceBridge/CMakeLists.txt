# FreedomServiceBridge — JNI marshalling for FreedomService.
cmake_minimum_required(VERSION 3.22.1)

project(bridge_freedom_service LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(bridge_freedom_service STATIC bridge_freedom_service.cpp)

target_include_directories(bridge_freedom_service PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_link_libraries(bridge_freedom_service PRIVATE freedom_service common)
