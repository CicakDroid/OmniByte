# engine-core/Shared — Shared types and interfaces for cross-module communication
cmake_minimum_required(VERSION 3.22.1)

# Collect all source files
set(SHARED_SOURCES
    MetadataStore.cpp
    MetadataSearchPipeline.cpp
)

set(SHARED_HEADERS
    IMetadataStore.h
    MetadataEntry.h
    MetadataStore.h
    MetadataSearchPipeline.h
)

# Create static library
add_library(shared STATIC ${SHARED_SOURCES} ${SHARED_HEADERS})

target_include_directories(shared
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(shared PUBLIC cxx_std_17)
