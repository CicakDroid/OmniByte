cmake_minimum_required(VERSION 3.22.1)
project(bypasser_adapter LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Upstream version (bump when updating) ---
# Source: https://github.com/LRFP-Team/Bypasser (MIT)
# Userspace environment detection bypass for Android.
set(BYPASSER_VERSION "1.0.0")

# --- Adapter library ---
# No FetchContent — Bypasser is userspace-only, uses dlsym + hooking.
# References:
#   - LRFP-Team/Bypasser: https://github.com/LRFP-Team/Bypasser

add_library(bypasser_adapter STATIC
    BypasserAdapter.cpp
)

target_include_directories(bypasser_adapter PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..    # ZigZag root (IStealthBackend.h)
)

target_link_libraries(bypasser_adapter PRIVATE log dl)

# --- Version check and updater ---
function(check_and_update_bypasser)
    set(CURRENT_VERSION ${BYPASSER_VERSION})
    message(STATUS "BypasserAdapter version: ${CURRENT_VERSION}")

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.bypasser_version")
        file(READ "${CMAKE_CURRENT_SOURCE_DIR}/.bypasser_version" STORED_VERSION)
        if(STORED_VERSION VERSION_LESS CURRENT_VERSION)
            message(STATUS "BypasserAdapter update available: ${STORED_VERSION} -> ${CURRENT_VERSION}")
            file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/.bypasser_version" "${CURRENT_VERSION}")
        else()
            message(STATUS "BypasserAdapter is up to date: ${STORED_VERSION}")
        endif()
    else()
        message(STATUS "First build of BypasserAdapter v${CURRENT_VERSION}")
        file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/.bypasser_version" "${CURRENT_VERSION}")
    endif()
endfunction()

# Run version check
check_and_update_bypasser()
