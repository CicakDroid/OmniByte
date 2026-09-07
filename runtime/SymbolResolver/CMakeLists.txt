# SymbolResolver — cross-process symbol resolution via xDL.
cmake_minimum_required(VERSION 3.22.1)

project(symbols_xdl LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(symbols_xdl STATIC SymbolResolver.cpp)

target_include_directories(symbols_xdl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)

# xDL library — fetch from GitHub hexhacking/xDL.
include(FetchContent)
FetchContent_Declare(
    xdl
    GIT_REPOSITORY https://github.com/hexhacking/xDL.git
    GIT_TAG        main
)
FetchContent_MakeAvailable(xdl)
target_link_libraries(symbols_xdl PRIVATE xdl)
