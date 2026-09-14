cmake_minimum_required(VERSION 3.22.1)
project(bhook_adapter LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Upstream version (bump when updating) ---
set(BHOOK_VERSION "1.1.2")

# --- Fetch upstream ---
include(FetchContent)
FetchContent_Declare(
    bhook
    GIT_REPOSITORY https://github.com/bytedance/bhook
    GIT_TAG        v${BHOOK_VERSION}
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(bhook)

# --- Adapter library ---
add_library(bhook_adapter STATIC BhookAdapter.cpp)

target_include_directories(bhook_adapter PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..    # HookEngines root (IHookEngines.h)
)
target_link_libraries(bhook_adapter PRIVATE log bytehook)
