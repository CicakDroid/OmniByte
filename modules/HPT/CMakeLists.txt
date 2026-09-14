# HPT — Hooking Platform Toolkit orchestrator.
cmake_minimum_required(VERSION 3.22.1)

project(hpt LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Each adapter is built via its own CMakeLists.txt (FetchContent + version check).
# This top-level HPT library links them together.

add_subdirectory(Hooker/HookEngines/ARTHook/Albatross)
add_subdirectory(Hooker/HookEngines/PLT_GOTHook/Bhook)
add_subdirectory(Hooker/HookEngines/InlineHook/Shadowhook)
add_subdirectory(Hooker/HookEngines/TracelessHook/Vectorhook)
add_subdirectory(MemoryEditor/KittyMemory)

add_library(hpt STATIC
    HPT.cpp
    Hooker/Hooking.cpp
    MemoryEditor/MemoryEditing.cpp
)

target_include_directories(hpt PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_link_libraries(hpt
    PRIVATE dumper common log
    PUBLIC  albatross_adapter bhook_adapter shadowhook_adapter
            vectorhook_adapter kittymemory_adapter kittymemoryex_adapter
)
