cmake_minimum_required(VERSION 3.22.1)
project(crc_adapter LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Upstream references (no FetchContent — ART/JNI hooking via dlsym) ---
# Sources:
#   - aimardcr/APKKiller (458 stars)
#     https://github.com/aimardcr/APKKiller
#     Bypasses APK signature verification using JNI + reflection.
#   - L-JINBIN/ApkSignatureKiller (971 stars)
#     https://github.com/L-JINBIN/ApkSignatureKiller
#     Hooks PackageManager.getPackageInfo() via ART JNI interception.
#   - Android Checksum API:
#     https://developer.android.com/reference/android/content/pm/Checksum
#     TYPE_WHOLE_MERKLE_ROOT_4K_SHA256, TYPE_WHOLE_MD5, etc.
#
# Technique: Hook art::JNI::CallObjectMethodV in libart.so to intercept
# PackageManager calls and return spoofed signatures/checksums.
set(CRC_VERSION "1.0.0")

# --- Adapter library ---
add_library(crc_adapter STATIC
    Crc.cpp
)

target_include_directories(crc_adapter PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../..    # ZigZag root (IStealthBackend.h)
)

target_link_libraries(crc_adapter PRIVATE log dl)

# --- Version check and updater ---
function(check_and_update_crc)
    set(CURRENT_VERSION ${CRC_VERSION})
    message(STATUS "CrcAdapter version: ${CURRENT_VERSION}")

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.crc_version")
        file(READ "${CMAKE_CURRENT_SOURCE_DIR}/.crc_version" STORED_VERSION)
        if(STORED_VERSION VERSION_LESS CURRENT_VERSION)
            message(STATUS "CrcAdapter update: ${STORED_VERSION} -> ${CURRENT_VERSION}")
            file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/.crc_version" "${CURRENT_VERSION}")
        else()
            message(STATUS "CrcAdapter up to date: ${STORED_VERSION}")
        endif()
    else()
        message(STATUS "First build of CrcAdapter v${CURRENT_VERSION}")
        file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/.crc_version" "${CURRENT_VERSION}")
    endif()
endfunction()

check_and_update_crc()
