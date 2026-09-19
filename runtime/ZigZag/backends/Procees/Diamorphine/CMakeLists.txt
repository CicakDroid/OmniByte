cmake_minimum_required(VERSION 3.22.1)
project(diamorphine_adapter LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Upstream version (bump when updating) ---
# Source: https://github.com/m0nad/Diamorphine (GPL-2.0)
# LKM rootkit for process/module hiding via getdents64 hook.
set(DIAMORPHINE_VERSION "1.0.0")

# --- Adapter library ---
# No FetchContent — Diamorphine is a kernel module, not userspace lib.
# This adapter communicates via ioctl to the loaded module.
# References:
#   - m0nad/Diamorphine: https://github.com/m0nad/Diamorphine

add_library(diamorphine_adapter STATIC
    DiamorphineAdapter.cpp
)

target_include_directories(diamorphine_adapter PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..    # ZigZag root (IStealthBackend.h)
)

target_link_libraries(diamorphine_adapter PRIVATE log dl)

# --- Version check and updater ---
function(check_and_update_diamorphine)
    set(CURRENT_VERSION ${DIAMORPHINE_VERSION})
    message(STATUS "DiamorphineAdapter version: ${CURRENT_VERSION}")

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.diamorphine_version")
        file(READ "${CMAKE_CURRENT_SOURCE_DIR}/.diamorphine_version" STORED_VERSION)
        if(STORED_VERSION VERSION_LESS CURRENT_VERSION)
            message(STATUS "DiamorphineAdapter update available: ${STORED_VERSION} -> ${CURRENT_VERSION}")
            file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/.diamorphine_version" "${CURRENT_VERSION}")
        else()
            message(STATUS "DiamorphineAdapter is up to date: ${STORED_VERSION}")
        endif()
    else()
        message(STATUS "First build of DiamorphineAdapter v${CURRENT_VERSION}")
        file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/.diamorphine_version" "${CURRENT_VERSION}")
    endif()
endfunction()

# Run version check
check_and_update_diamorphine()
