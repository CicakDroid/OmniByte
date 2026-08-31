# config — runtime configuration system shared across OmniByte native components
# (engine-core, runtime, modules/Dumper). Header-only library; consumers link
# 'dumper_config' to get nlohmann/json + this directory on the include path.
cmake_minimum_required(VERSION 3.22.1)

project(dumper_config LANGUAGES CXX)

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
# so downstream can: target_link_libraries(... dumper_config)
add_library(dumper_config INTERFACE)
target_include_directories(dumper_config INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(dumper_config INTERFACE nlohmann_json::nlohmann_json)
