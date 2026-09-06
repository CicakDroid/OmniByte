cmake_minimum_required(VERSION 3.14)

# Signatures plugin — aggregates MagicBytes, Pattern, SignaturesDatabase, and Yara

# ── MagicBytes (always built) ──
add_library(signatures_magicbytes STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/MagicBytes/MagicBytesDetector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/MagicBytes/MagicBytesExtractor.cpp
)
target_include_directories(signatures_magicbytes PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/MagicBytes
)

# ── Pattern (always built) ──
add_library(signatures_pattern STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Pattern/PatternScanner.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Pattern/PatternMatcher.cpp
)
target_include_directories(signatures_pattern PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Pattern
)

# ── SignaturesDatabase (always built) ──
add_library(signatures_database STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/SignaturesDatabase/SignaturesDatabase.cpp
)
target_include_directories(signatures_database PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/SignaturesDatabase
)

# ── Yara (optional, controlled by ENABLE_YARA) ──
option(ENABLE_YARA "Enable YARA rule scanning" ON)

if(ENABLE_YARA)
    add_subdirectory(Yara)
endif()

# ── Main Signatures plugin (ties everything together) ──
add_library(signatures_plugin STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/plugin_signatures.cpp
)
target_include_directories(signatures_plugin PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(signatures_plugin
    PRIVATE
        signatures_magicbytes
        signatures_pattern
        signatures_database
)
if(ENABLE_YARA)
    target_link_libraries(signatures_plugin PRIVATE yara)
    target_compile_definitions(signatures_plugin PRIVATE HAS_YARA)
endif()
