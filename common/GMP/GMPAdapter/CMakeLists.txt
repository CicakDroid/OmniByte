# common/GMP/GMPAdapter — GNU Multiple Precision Arithmetic Library.
# Source: https://gmplib.org (LGPL v3)
# Version: 6.3.0
#
# Provides omnibyte_gmp STATIC target with gmp.h linked.
# Fetches GMP source and cross-compiles for Android NDK.
cmake_minimum_required(VERSION 3.22.1)
project(omnibyte_gmp LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# --- Upstream version (bump when updating) ---
set(GMP_VERSION "6.3.0")
set(GMP_URL "https://gmplib.org/download/gmp/gmp-${GMP_VERSION}.tar.xz")
set(GMP_URL_HASH "SHA256=a3c2b3ce6593c4898bb301548ee1e70f83bcc1878b4e00ebff5c6f87f1387a1e")

# --- Fetch upstream source ---
include(FetchContent)
cmake_policy(SET CMP0135 NEW)
FetchContent_Declare(
    gmp
    URL            ${GMP_URL}
    URL_HASH       ${GMP_URL_HASH}
    SOURCE_DIR     ${CMAKE_CURRENT_BINARY_DIR}/gmp-src
)

# GMP requires autotools; for Android NDK we use pre-configured static build.
# The configure/make step runs at build time via ExternalProject.
include(ExternalProject)
ExternalProject_Add(
    gmp_build
    URL            ${GMP_URL}
    URL_HASH       ${GMP_URL_HASH}
    SOURCE_DIR     ${CMAKE_CURRENT_BINARY_DIR}/gmp-build
    PREFIX         ${CMAKE_CURRENT_BINARY_DIR}/gmp-prefix
    CONFIGURE_COMMAND
        ${CMAKE_CURRENT_BINARY_DIR}/gmp-build/configure
        --host=aarch64-linux-android
        --prefix=${CMAKE_CURRENT_BINARY_DIR}/gmp-install
        --disable-shared
        --enable-static
        --enable-cxx
        CC=${CMAKE_C_COMPILER}
        CXX=${CMAKE_CXX_COMPILER}
        CFLAGS="${CMAKE_C_FLAGS}"
        CXXFLAGS="${CMAKE_CXX_FLAGS}"
    BUILD_COMMAND     make -j${NPROC}
    INSTALL_COMMAND   make install
    BUILD_IN_SOURCE   0
)

# --- GMP imported static library ---
add_library(omnibyte_gmp STATIC IMPORTED GLOBAL)
set_target_properties(omnibyte_gmp PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/gmp-install/lib/libgmp.a
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_BINARY_DIR}/gmp-install/include
)
add_dependencies(omnibyte_gmp gmp_build)

# --- Adapter library ---
add_library(omnibyte_gmp_adapter STATIC GMPAdapter.cpp)

target_include_directories(omnibyte_gmp_adapter PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..       # exposes <common/GMP/...>
    ${CMAKE_CURRENT_SOURCE_DIR}/../..    # exposes <common/...>
)
target_link_libraries(omnibyte_gmp_adapter
    PUBLIC  omnibyte_gmp
    PRIVATE log
)
add_dependencies(omnibyte_gmp_adapter gmp_build)

# Version info
target_compile_definitions(omnibyte_gmp_adapter PUBLIC
    GMP_VERSION_MAJOR=6
    GMP_VERSION_MINOR=3
    GMP_VERSION_PATCH=0
    OMNIBYTE_GMP_VERSION="6.3.0"
)
