# Enhanced/Emulation/Unicorn — CPU emulation wrapper via Unicorn Engine.
cmake_minimum_required(VERSION 3.22.1)

project(emulation_unicorn LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Unicorn Engine dependency ──────────────────────────────────
# Cari Unicorn via pkg-config atau find_library.
# Kalau tidak ketemu, build dalam mode stub (compile-only, runtime gagal).
find_path(UNICORN_INCLUDE_DIR unicorn/unicorn.h
    HINTS /usr/local/include /opt/unicorn/include
)
find_library(UNICORN_LIBRARY unicorn
    HINTS /usr/local/lib /opt/unicorn/lib
)

if(UNICORN_INCLUDE_DIR AND UNICORN_LIBRARY)
    message(STATUS "Unicorn Engine found: ${UNICORN_LIBRARY}")
    set(UNICORN_FOUND TRUE)
else()
    message(WARNING "Unicorn Engine NOT found — building in STUB mode "
                    "(compile OK, runtime will fail). "
                    "Install Unicorn v2.1.4: https://github.com/unicorn-engine/unicorn")
    set(UNICORN_FOUND FALSE)
endif()

# ── Library ────────────────────────────────────────────────────
add_library(emulation_unicorn STATIC
    unicorn_engine.cpp
)

target_include_directories(emulation_unicorn PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
)

if(UNICORN_FOUND)
    target_include_directories(emulation_unicorn PRIVATE ${UNICORN_INCLUDE_DIR})
    target_link_libraries(emulation_unicorn PRIVATE ${UNICORN_LIBRARY})
    target_compile_definitions(emulation_unicorn PRIVATE UNICORN_FOUND=1)
endif()
