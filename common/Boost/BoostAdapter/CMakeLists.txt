# common/Boost/BoostAdapter — Boost C++ libraries.
# Source: https://github.com/boostorg/boost (BSL-1.0)
# Version: 1.83.0
#
# Provides omnibyte_boost INTERFACE target with selected Boost headers.
# Fetches Boost source for header-only libraries we use.
cmake_minimum_required(VERSION 3.22.1)
project(omnibyte_boost LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Upstream version (bump when updating) ---
set(BOOST_VERSION "1.83.0")
set(BOOST_URL "https://github.com/boostorg/boost/releases/download/boost-1.83.0/boost-1.83.0-b2-nodocs.tar.gz")

# --- Fetch upstream ---
include(FetchContent)
cmake_policy(SET CMP0135 NEW)
FetchContent_Declare(
    boost
    URL            ${BOOST_URL}
    URL_HASH       SHA256=07247b157f3222e0b57af5630c4f4dd5c5120b12c0a65dd15f72b970c35fce29
    SOURCE_DIR     ${CMAKE_CURRENT_BINARY_DIR}/boost-src
)

# Boost is header-only for the subset we use; just fetch headers.
FetchContent_MakeAvailable(boost)

# --- Adapter library (header-only target) ---
add_library(omnibyte_boost INTERFACE)

target_include_directories(omnibyte_boost INTERFACE
    ${CMAKE_CURRENT_BINARY_DIR}/boost-src        # Boost root
    ${CMAKE_CURRENT_SOURCE_DIR}/..               # exposes <common/Boost/...>
    ${CMAKE_CURRENT_SOURCE_DIR}/../..            # exposes <common/...>
)

# Version info
target_compile_definitions(omnibyte_boost INTERFACE
    BOOST_VERSION_MAJOR=1
    BOOST_VERSION_MINOR=83
    BOOST_VERSION_PATCH=0
    OMNIBYTE_BOOST_VERSION="1.83.0"
)
