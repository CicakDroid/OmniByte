cmake_minimum_required(VERSION 3.22.1)
project(sqlhook_adapter LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Upstream version (bump when updating) ---
set(SQLHOOK_VERSION "1.0.0")

# --- Fetch upstream (SQLite) ---
include(FetchContent)
FetchContent_Declare(
    sqlite
    URL https://www.sqlite.org/2024/sqlite-amalgamation-3450000.zip
    # re-pinned: verified 3x across www+www2 (zip integrity, SQLITE_VERSION 3.45.0); old pin matched neither SHA256 nor SHA3-256 of the served file
    URL_HASH SHA256=bde30d13ebdf84926ddd5e8b6df145be03a577a48fd075a087a5dd815bcdf740
)
FetchContent_MakeAvailable(sqlite)

# --- Adapter library ---
add_library(sqlhook_adapter STATIC
    SQLhookAdapter.cpp
    ${sqlite_SOURCE_DIR}/sqlite3.c
)

target_include_directories(sqlhook_adapter PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..    # HookEngines root (IHookEngines.h)
    ${sqlite_SOURCE_DIR}
)

target_link_libraries(sqlhook_adapter PRIVATE log dl)

# --- Version check and updater ---
function(check_and_update_sqlhook)
    set(CURRENT_VERSION ${SQLHOOK_VERSION})
    message(STATUS "SQLhookAdapter version: ${CURRENT_VERSION}")

    # Check if update is needed
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.sqlhook_version")
        file(READ "${CMAKE_CURRENT_SOURCE_DIR}/.sqlhook_version" STORED_VERSION)
        if(STORED_VERSION VERSION_LESS CURRENT_VERSION)
            message(STATUS "SQLhookAdapter update available: ${STORED_VERSION} -> ${CURRENT_VERSION}")
            file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/.sqlhook_version" "${CURRENT_VERSION}")
        else()
            message(STATUS "SQLhookAdapter is up to date: ${STORED_VERSION}")
        endif()
    else()
        message(STATUS "First build of SQLhookAdapter v${CURRENT_VERSION}")
        file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/.sqlhook_version" "${CURRENT_VERSION}")
    endif()
endfunction()

# Run version check
check_and_update_sqlhook()
