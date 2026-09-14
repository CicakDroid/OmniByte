cmake_minimum_required(VERSION 3.22.1)
project(vectorhook_adapter LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Upstream version (bump when updating) ---
set(VECTORHOOK_VERSION "0.1.0")

# --- Fetch upstream (stealth-core) ---
include(FetchContent)
FetchContent_Declare(
    stealth_core
    GIT_REPOSITORY https://github.com/1013503897/stealth-core
    GIT_TAG        v${VECTORHOOK_VERSION}
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(stealth_core)

# --- Adapter library ---
add_library(vectorhook_adapter STATIC VectorhookAdapter.cpp)

target_include_directories(vectorhook_adapter PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..    # HookEngines root (IHookEngines.h)
)
target_link_libraries(vectorhook_adapter PRIVATE log)
