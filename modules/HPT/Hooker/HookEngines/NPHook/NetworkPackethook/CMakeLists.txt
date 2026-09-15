cmake_minimum_required(VERSION 3.22.1)
project(np_hook_adapter LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Upstream version (bump when updating) ---
set(NPHOOK_VERSION "1.0.0")

# --- Adapter library ---
# No FetchContent needed — hooks libc.so network functions via dlsym.
# References:
#   - sowmiksudo/android-tcp-sniffer (MIT) — libc.so hooking pattern
#     https://github.com/sowmiksudo/android-tcp-sniffer
#   - POSIX.1-2008 standard: send(), recv(), read(), write(), connect(), socket()

add_library(np_hook_adapter STATIC
    NetworkPackethook/NetworkPacketHookAdapter.cpp
)

target_include_directories(np_hook_adapter PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../..    # HookEngines root (IHookEngines.h)
)

target_link_libraries(np_hook_adapter PRIVATE log dl)

# --- Version check and updater ---
function(check_and_update_nphook)
    set(CURRENT_VERSION ${NPHOOK_VERSION})
    message(STATUS "NPhookAdapter version: ${CURRENT_VERSION}")

    # Check if update is needed
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.nphook_version")
        file(READ "${CMAKE_CURRENT_SOURCE_DIR}/.nphook_version" STORED_VERSION)
        if(STORED_VERSION VERSION_LESS CURRENT_VERSION)
            message(STATUS "NPhookAdapter update available: ${STORED_VERSION} -> ${CURRENT_VERSION}")
            file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/.nphook_version" "${CURRENT_VERSION}")
        else()
            message(STATUS "NPhookAdapter is up to date: ${STORED_VERSION}")
        endif()
    else()
        message(STATUS "First build of NPhookAdapter v${CURRENT_VERSION}")
        file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/.nphook_version" "${CURRENT_VERSION}")
    endif()
endfunction()

# Run version check
check_and_update_nphook()
