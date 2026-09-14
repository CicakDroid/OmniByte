cmake_minimum_required(VERSION 3.22.1)
project(shadowhook_adapter LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Upstream version (bump when updating) ---
set(SHADOWHOOK_VERSION "2.0.1")

# --- Fetch upstream ---
include(FetchContent)
FetchContent_Declare(
    shadowhook
    GIT_REPOSITORY https://github.com/bytedance/android-inline-hook
    GIT_TAG        v${SHADOWHOOK_VERSION}
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(shadowhook)

# --- Adapter library ---
add_library(shadowhook_adapter STATIC ShadowhookAdapter.cpp)

target_include_directories(shadowhook_adapter PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..    # HookEngines root (IHookEngines.h)
)
target_link_libraries(shadowhook_adapter PRIVATE log shadowhook)
