cmake_minimum_required(VERSION 3.22.1)
project(kittymemory_adapter LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Upstream versions (bump when updating) ---
set(KITTYMEMORY_VERSION  "2.0")
set(KITTYMEMORYEX_VERSION "2.0")

# --- Fetch upstream ---
include(FetchContent)

FetchContent_Declare(
    kittymemory
    GIT_REPOSITORY https://github.com/MJx0/KittyMemory
    GIT_TAG        master   # no tags available
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(kittymemory)

FetchContent_Declare(
    kittymemoryex
    GIT_REPOSITORY https://github.com/MJx0/KittyMemoryEx
    GIT_TAG        main     # no tags available
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(kittymemoryex)

# --- Adapter: KittyMemory (in-process) ---
add_library(kittymemory_adapter STATIC KittyMemoryAdapter.cpp)

target_include_directories(kittymemory_adapter PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..    # HPT root (MemoryEditor.h)
    ${kittymemory_SOURCE_DIR}
)
target_link_libraries(kittymemory_adapter PRIVATE log)

# --- Adapter: KittyMemoryEx (remote process) ---
add_library(kittymemoryex_adapter STATIC KittyMemoryExAdapter.cpp)

target_include_directories(kittymemoryex_adapter PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..    # HPT root (MemoryEditor.h)
    ${kittymemoryex_SOURCE_DIR}
)
target_link_libraries(kittymemoryex_adapter PRIVATE log)
