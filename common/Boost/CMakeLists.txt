# common/Boost — Boost C++ libraries.
# Source: https://github.com/boostorg/boost (BSL-1.0)
# Version: 1.92.0
#
# Provides omnibyte_boost INTERFACE (headers) + omnibyte_boost_adapter STATIC.
# Fetches Boost source for header-only libraries we use.
cmake_minimum_required(VERSION 3.22.1)
project(omnibyte_boost LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Upstream version (bump when updating) ---
set(BOOST_VERSION "1.92.0")
set(BOOST_URL "https://github.com/boostorg/boost/releases/download/boost-1.92.0/boost-1.92.0-b2-nodocs.tar.gz")

# --- Fetch upstream ---
include(FetchContent)
cmake_policy(SET CMP0135 NEW)
FetchContent_Declare(
    boost
    URL            ${BOOST_URL}
    URL_HASH       SHA256=e2a814b3a158ab482c7a3d330f8bf5a7a8423d258a4e2fb396996e00fcee2111
    SOURCE_DIR     ${CMAKE_CURRENT_BINARY_DIR}/boost-src
)

# Boost is header-only for the subset we use; just fetch headers.
FetchContent_MakeAvailable(boost)

# --- Boost extension: aho_corasick (header-only, NOT upstream Boost) ---
# Populate-only: skip vendor CMake (tests/benchmarks); we only need headers.
set(AHO_CORASICK_GIT_TAG "b4fb360ef2764bf0cd22daaa62dd4bfd18b8ad0c")
FetchContent_Declare(
    aho_corasick
    GIT_REPOSITORY https://github.com/cjgdev/aho_corasick.git
    GIT_TAG        ${AHO_CORASICK_GIT_TAG}
    GIT_SHALLOW    FALSE
)
FetchContent_GetProperties(aho_corasick)
if(NOT aho_corasick_POPULATED)
    FetchContent_Populate(aho_corasick)
endif()
# Vendor layout: <root>/src/aho_corasick/aho_corasick.hpp

# --- Boost extension: libcuckoo (header-only, NOT upstream Boost) ---
# Populate-only: skip vendor CMake (tests/examples); we only need headers.
set(LIBCUCKOO_GIT_TAG "6a2555d551b7703d7176a5d114219a02c55d4038")
FetchContent_Declare(
    libcuckoo
    GIT_REPOSITORY https://github.com/efficient/libcuckoo.git
    GIT_TAG        ${LIBCUCKOO_GIT_TAG}
    GIT_SHALLOW    FALSE
)
FetchContent_GetProperties(libcuckoo)
if(NOT libcuckoo_POPULATED)
    FetchContent_Populate(libcuckoo)
endif()
# Vendor layout: <root>/libcuckoo/cuckoohash_map.hh → include as <libcuckoo/...>

# --- Header-only Boost root ---
add_library(omnibyte_boost INTERFACE)

target_include_directories(omnibyte_boost INTERFACE
    ${CMAKE_CURRENT_BINARY_DIR}/boost-src        # Boost root (<boost/...>)
    ${aho_corasick_SOURCE_DIR}/src               # <aho_corasick/aho_corasick.hpp>
    ${libcuckoo_SOURCE_DIR}                      # <libcuckoo/cuckoohash_map.hh>
    ${CMAKE_CURRENT_SOURCE_DIR}                  # BoostAdapter.h in this dir
    ${CMAKE_CURRENT_SOURCE_DIR}/..               # exposes <Boost/...>
    ${CMAKE_CURRENT_SOURCE_DIR}/../..            # exposes <common/Boost/...>
)

target_compile_definitions(omnibyte_boost INTERFACE
    BOOST_VERSION_MAJOR=1
    BOOST_VERSION_MINOR=92
    BOOST_VERSION_PATCH=0
    OMNIBYTE_BOOST_VERSION="1.92.0"
)

# --- Adapter library ---
add_library(omnibyte_boost_adapter STATIC
    BoostAdapter.cpp
    PatternScanner.cpp
    AhoCorasick.cpp
    CuckooHash.cpp
    Graph/Graph.cpp
)

target_include_directories(omnibyte_boost_adapter PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..               # exposes <Boost/...>
    ${CMAKE_CURRENT_SOURCE_DIR}/../..            # exposes <common/Boost/...>
)
target_link_libraries(omnibyte_boost_adapter
    PUBLIC  omnibyte_boost
            omnibyte_highway
    PRIVATE log
)
