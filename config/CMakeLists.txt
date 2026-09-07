# config — runtime configuration system shared across OmniByte native components
# (engine-core, runtime, modules/Dumper). Provides ConfigManager singleton and
# 7 typed config structs with JSON serialization.
cmake_minimum_required(VERSION 3.22.1)

project(dumper_config LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- nlohmann/json via omnibyte_serialization (single-source-of-truth) ---
include(${CMAKE_CURRENT_SOURCE_DIR}/../common/Serialization/CMakeLists.txt)

add_library(dumper_config STATIC
    ConfigManager.cpp
)
target_include_directories(dumper_config PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../common
)
target_link_libraries(dumper_config PUBLIC omnibyte_serialization)
