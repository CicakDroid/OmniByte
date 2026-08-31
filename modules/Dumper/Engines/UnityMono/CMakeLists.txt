# Dumper Engine: UnityMono — Analyzer + Resolver + Profiles.
cmake_minimum_required(VERSION 3.22.1)

project(engine_unitymono LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(Analyzer)
add_subdirectory(Resolver)
add_subdirectory(Profiles)

# Combined static library for the UnityMono engine
add_library(engine_unitymono STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Analyzer/unitymono_analyzer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Resolver/unitymono_resolver.cpp
)

target_include_directories(engine_unitymono PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Analyzer
    ${CMAKE_CURRENT_SOURCE_DIR}/Resolver
    ${CMAKE_CURRENT_SOURCE_DIR}/Profiles
)

target_link_libraries(engine_unitymono PUBLIC
    unitymono_analyzer
    unitymono_resolver
    unitymono_profiles
)
