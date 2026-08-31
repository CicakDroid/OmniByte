# xdl-adapter — resolves dlopen/dlsym consistently across Android versions.
#
# Stage 1 scaffolding only. Real backend wiring is commented out below.
cmake_minimum_required(VERSION 3.22.1)

project(symbols_xdl LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(symbols_xdl STATIC symbols_xdl.cpp)

# ===== PLACEHOLDER — real xdl backend wiring (uncomment when vendored) =====
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendored/xdl xdl)
# target_include_directories(symbols_xdl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/vendored/xdl)
# target_link_libraries(symbols_xdl PUBLIC xdl)
