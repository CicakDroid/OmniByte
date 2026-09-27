# common/TaskFlow — parallel task execution via taskflow/taskflow.
# Static adapter; provides omnibyte_taskflow target.
cmake_minimum_required(VERSION 3.22.1)
project(omnibyte_taskflow LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Upstream version (bump when updating) ---
set(TASKFLOW_VERSION "3.8.0")

# --- Fetch upstream ---
# Taskflow's examples define an executable named "runtime" which collides with
# our runtime target; force tests/examples off before MakeAvailable.
set(TF_BUILD_TESTS OFF CACHE BOOL "Taskflow tests" FORCE)
set(TF_BUILD_EXAMPLES OFF CACHE BOOL "Taskflow examples" FORCE)
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
add_library(omnibyte_taskflow STATIC
    TaskFlow.cpp
    TaskFlowAdapter.cpp
)

target_include_directories(omnibyte_taskflow PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..       # exposes <common/TaskFlow/...>
    ${CMAKE_CURRENT_SOURCE_DIR}/../..    # exposes <common/...>
)
target_link_libraries(omnibyte_taskflow
    PUBLIC  Taskflow  # v3.8.0 target name (no tf::tf alias exists)
    PRIVATE log
)
