cmake_minimum_required(VERSION 3.22.1)
project(albatross_adapter LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Upstream version (bump when updating) ---
set(ALBATROSS_VERSION "3.6.0")

# --- Fetch upstream ---
include(FetchContent)
FetchContent_Declare(
    albatross_android
    GIT_REPOSITORY https://github.com/AlbatrossHook/AlbatrossAndroid
    GIT_TAG        ${ALBATROSS_VERSION}
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(albatross_android)

# --- Adapter library ---
add_library(albatross_adapter STATIC AlbatrossAdapter.cpp)

target_include_directories(albatross_adapter PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..    # HookEngines root (IHookEngines.h)
)
target_link_libraries(albatross_adapter PRIVATE log jnigraphics)
