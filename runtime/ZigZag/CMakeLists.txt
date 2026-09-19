# ZigZag — stealth backends (Diamorphine, Bypasser, Pairip, Crc).
cmake_minimum_required(VERSION 3.22.1)

project(zigzag LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Sub-adapter libraries (each with own CMakeLists.txt + version check) ---
add_subdirectory(backends/Procees/Diamorphine)
add_subdirectory(backends/LRFP/Bypasser)
add_subdirectory(backends/Pairip)
add_subdirectory(backends/CRC)

add_library(zigzag STATIC
    ZigZag.cpp
    backends/Procees/Diamorphine/DiamorphineAdapter.cpp
    backends/Procees/HideProcess.cpp
    backends/LRFP/Bypasser/BypasserAdapter.cpp
    backends/LRFP/LRFP.cpp
    backends/Pairip/Pairip.cpp
    backends/CRC/Crc.cpp
)

target_include_directories(zigzag PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_link_libraries(zigzag PRIVATE common diamorphine_adapter bypasser_adapter pairip_adapter crc_adapter)
