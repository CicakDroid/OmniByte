cmake_minimum_required(VERSION 3.22.1)
project(io_hook_adapter LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Upstream version (bump when updating) ---
set(IOHOOK_VERSION "1.0.0")

# --- Adapter library ---
# No FetchContent needed — hooks libc.so I/O functions via dlsym.
# References:
#   - POSIX.1-2008 standard: openat(), open(), fopen(), access(), stat(), unlink()

add_library(io_hook_adapter STATIC
    IoHookAdapter.cpp
)

target_include_directories(io_hook_adapter PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../..    # HookEngines root (IHookEngines.h)
)

target_link_libraries(io_hook_adapter PRIVATE log dl)

# --- Version check and updater ---
function(check_and_update_iohook)
    set(CURRENT_VERSION ${IOHOOK_VERSION})
    message(STATUS "IoHookAdapter version: ${CURRENT_VERSION}")

    # Check if update is needed
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.iohook_version")
        file(READ "${CMAKE_CURRENT_SOURCE_DIR}/.iohook_version" STORED_VERSION)
        if(STORED_VERSION VERSION_LESS CURRENT_VERSION)
            message(STATUS "IoHookAdapter update available: ${STORED_VERSION} -> ${CURRENT_VERSION}")
            file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/.iohook_version" "${CURRENT_VERSION}")
        else()
            message(STATUS "IoHookAdapter is up to date: ${STORED_VERSION}")
        endif()
    else()
        message(STATUS "First build of IoHookAdapter v${CURRENT_VERSION}")
        file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/.iohook_version" "${CURRENT_VERSION}")
    endif()
endfunction()

# Run version check
check_and_update_iohook()
