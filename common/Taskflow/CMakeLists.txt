# common/Taskflow — parallel task execution via taskflow/taskflow.
# Header-only library; provides omnibyte_taskflow INTERFACE target.
cmake_minimum_required(VERSION 3.22.1)
project(omnibyte_taskflow LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Upstream version (bump when updating) ---
set(TASKFLOW_VERSION "3.8.0")

# --- Fetch upstream ---
include(FetchContent)
cmake_policy(SET CMP0135 NEW)
FetchContent_Declare(
    taskflow
    GIT_REPOSITORY https://github.com/taskflow/taskflow.git
    GIT_TAG        v${TASKFLOW_VERSION}
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(taskflow)

# --- Adapter library ---
add_library(omnibyte_taskflow STATIC TaskflowAdapter.cpp)

target_include_directories(omnibyte_taskflow PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..       # exposes <common/Taskflow/...>
    ${CMAKE_CURRENT_SOURCE_DIR}/../..    # exposes <common/...>
)
target_link_libraries(omnibyte_taskflow
    PUBLIC  tf::tf    # taskflow header-only target
    PRIVATE log
)
