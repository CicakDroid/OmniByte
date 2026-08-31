# config — runtime configuration system shared across OmniByte native components
# (engine-core, runtime, modules/Dumper). Header-only library; consumers link
# 'dumper_config' to get nlohmann/json + this directory on the include path.
cmake_minimum_required(VERSION 3.22.1)

project(dumper_config LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- nlohmann/json via omnibyte_serialization (single-source-of-truth) ---
# nlohmann/json is fetched once in common/Serialization/CMakeLists.txt.
# We re-use that target here so there is zero duplicate fetch.
include(${CMAKE_CURRENT_SOURCE_DIR}/../common/Serialization/CMakeLists.txt)

# Header-only — no static library needed, but provide an INTERFACE target
# so downstream can: target_link_libraries(... dumper_config)
add_library(dumper_config INTERFACE)
target_include_directories(dumper_config INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../common
)
target_link_libraries(dumper_config INTERFACE omnibyte_serialization)
