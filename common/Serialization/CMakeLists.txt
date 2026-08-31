# common/Serialization — nlohmann/json single-source-of-truth target
# Provides omnibyte_serialization INTERFACE library that exposes nlohmann::json.
# All consumers (dumper_config, Export/Writers, HydraDis) link this instead
# of fetching nlohmann independently.
if(NOT TARGET omnibyte_serialization)
cmake_minimum_required(VERSION 3.22.1)

project(omnibyte_serialization LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- nlohmann/json via FetchContent (single-header) ---
include(FetchContent)
cmake_policy(SET CMP0135 NEW)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
    GIT_SHALLOW    TRUE
)
set(JSON_BuildTests OFF CACHE INTERNAL "")
set(JSON_Install OFF CACHE INTERNAL "")
FetchContent_MakeAvailable(nlohmann_json)

# Header-only — no static library needed, but provide an INTERFACE target
# so downstream can: target_link_libraries(... omnibyte_serialization)
add_library(omnibyte_serialization INTERFACE)
target_include_directories(omnibyte_serialization INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/..          # exposes <common/Serialization/…>
    ${CMAKE_CURRENT_SOURCE_DIR}/../..       # exposes <common/…> if needed later
)
target_link_libraries(omnibyte_serialization INTERFACE nlohmann_json::nlohmann_json)
endif()
