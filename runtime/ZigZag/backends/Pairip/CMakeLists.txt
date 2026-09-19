cmake_minimum_required(VERSION 3.22.1)
project(pairip_adapter LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Upstream references (no FetchContent — userspace hooking via dlsym) ---
# Sources:
#   - KOWX712/PlayIntegrityFix (GPL-3.0, 3.8k stars)
#     https://github.com/KOWX712/PlayIntegrityFix
#     Zygisk module: spoofs device props + keybox for MEETS_DEVICE_INTEGRITY.
#   - dpejoh/specter (GPL-3.0)
#     https://github.com/dpejoh/specter
#     Unified Play Integrity + root hiding stack.
#   - chiteroman/PlayIntegrityFix (original, archived)
#     https://github.com/chiteroman/PlayIntegrityFix
#
# Technique: __system_property_get hook via PLT patching (HPT subsystem).
# No kernel module needed — userspace only.
set(PAIRIP_VERSION "1.0.0")

# --- Adapter library ---
add_library(pairip_adapter STATIC
    Pairip.cpp
)

target_include_directories(pairip_adapter PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../..    # ZigZag root (IStealthBackend.h)
)

target_link_libraries(pairip_adapter PRIVATE log dl)

# --- Version check and updater ---
function(check_and_update_pairip)
    set(CURRENT_VERSION ${PAIRIP_VERSION})
    message(STATUS "PairipAdapter version: ${CURRENT_VERSION}")

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.pairip_version")
        file(READ "${CMAKE_CURRENT_SOURCE_DIR}/.pairip_version" STORED_VERSION)
        if(STORED_VERSION VERSION_LESS CURRENT_VERSION)
            message(STATUS "PairipAdapter update: ${STORED_VERSION} -> ${CURRENT_VERSION}")
            file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/.pairip_version" "${CURRENT_VERSION}")
        else()
            message(STATUS "PairipAdapter up to date: ${STORED_VERSION}")
        endif()
    else()
        message(STATUS "First build of PairipAdapter v${CURRENT_VERSION}")
        file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/.pairip_version" "${CURRENT_VERSION}")
    endif()
endfunction()

check_and_update_pairip()
