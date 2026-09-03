cmake_minimum_required(VERSION 3.14)

option(ENABLE_YARA "Enable YARA rule scanning (requires libyara)" OFF)

if(NOT ENABLE_YARA)
    add_library(yarascan_stub STATIC plugin_yarascan_stub.cpp)
    target_include_directories(yarascan_stub PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
    return()
endif()

find_package(yara REQUIRED)

add_library(yarascan STATIC
    YaraScan.cpp
    plugin_yarascan.cpp
)

target_include_directories(yarascan PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(yarascan PRIVATE yara)
target_compile_definitions(yarascan PRIVATE HAS_YARA)
