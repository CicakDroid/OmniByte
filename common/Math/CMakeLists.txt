# common/Math — GLM (OpenGL Mathematics) single-source-of-truth target
# Provides omnibyte_math INTERFACE library that exposes glm/glm.hpp.
# Prepares for Engines/*/Analyzer to parse transforms/matrices later.
# No wrapper/helper needed yet — just the CMake wiring.
if(NOT TARGET omnibyte_math)
cmake_minimum_required(VERSION 3.22.1)

project(omnibyte_math LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- GLM via FetchContent (header-only) ---
include(FetchContent)
cmake_policy(SET CMP0135 NEW)
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(glm)

# Header-only — provide INTERFACE target
# so downstream can: target_link_libraries(... omnibyte_math)
add_library(omnibyte_math INTERFACE)
target_include_directories(omnibyte_math INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/..          # exposes <common/Math/…> if needed later
    ${CMAKE_CURRENT_SOURCE_DIR}/../..       # exposes <common/…> if needed later
)
target_link_libraries(omnibyte_math INTERFACE glm::glm)
endif()
