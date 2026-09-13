# Enhanced/Crypt plugin — crypto constant & pattern scanner.
cmake_minimum_required(VERSION 3.22.1)

project(enhanced_crypt LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# FindCrypt3 library
add_subdirectory(FindCrypt3)

# DeCrypt3 library
add_subdirectory(DeCrypt3)

# Plugin wrapper
add_library(enhanced_findcrypt STATIC enhanced_findcrypt.cpp)
target_include_directories(enhanced_findcrypt PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
)

add_library(enhanced_decrypt STATIC enhanced_decrypt.cpp)
target_include_directories(enhanced_decrypt PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
)
target_link_libraries(enhanced_decrypt PRIVATE decrypt3)
