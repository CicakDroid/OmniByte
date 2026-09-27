# common/GMP — GNU Multiple Precision Arithmetic Library.
# Source: https://gmplib.org (LGPL v3)
# Version: 6.3.0
#
# Provides omnibyte_gmp STATIC target with gmp.h/gmpxx.h linked.
# Fetches GMP source and cross-compiles for Android NDK.
# TODO: no vendored per-ABI prebuilt .a — relies on ExternalProject at build time
# (needs network). If offline builds matter, commit gmp.a per ABI under prebuilt/.
cmake_minimum_required(VERSION 3.22.1)
project(omnibyte_gmp LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# --- Upstream version (bump when updating) ---
set(GMP_VERSION "6.3.0")
set(GMP_URL "https://gmplib.org/download/gmp/gmp-${GMP_VERSION}.tar.xz")
set(GMP_URL_HASH "SHA256=a3c2b80201b89e68616f4ad30bc66aee4927c3ce50e33929ca819d5c43538898")

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
# NDK clang wrappers inject --target/--sysroot; raw clang fails configure
# (crtbeginS.o / -lgcc not found without them). Wrappers live beside CMAKE_C_COMPILER.
get_filename_component(_gmp_ndk_bin "${CMAKE_C_COMPILER}" DIRECTORY)
set(GMP_CC "${_gmp_ndk_bin}/aarch64-linux-android24-clang")
set(GMP_CXX "${_gmp_ndk_bin}/aarch64-linux-android24-clang++")
if(NOT EXISTS "${GMP_CC}")
    set(GMP_CC "${CMAKE_C_COMPILER}")
    set(GMP_CXX "${CMAKE_CXX_COMPILER}")
endif()

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
        --disable-assembly
        --disable-profiling
        CC=${GMP_CC}
        CXX=${GMP_CXX}
        CFLAGS=${CMAKE_C_FLAGS}
        CXXFLAGS=-O2\ -static-libstdc++
        LDFLAGS=-static-libstdc++
        MAKEINFO=true
    BUILD_COMMAND     ${CMAKE_MAKE_PROGRAM} -j
    INSTALL_COMMAND   ${CMAKE_MAKE_PROGRAM} install
    BUILD_IN_SOURCE   0
)

# --- GMP imported static libraries (C + C++ for gmpxx) ---
# gmp-install/ only exists after gmp_build runs; create the include dir at
# configure time so IMPORTED INTERFACE_INCLUDE_DIRECTORIES is valid.
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/gmp-install/include)
add_library(omnibyte_gmp STATIC IMPORTED GLOBAL)
set_target_properties(omnibyte_gmp PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/gmp-install/lib/libgmp.a
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_BINARY_DIR}/gmp-install/include
)
add_library(omnibyte_gmpxx STATIC IMPORTED GLOBAL)
set_target_properties(omnibyte_gmpxx PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/gmp-install/lib/libgmpxx.a
)
add_dependencies(omnibyte_gmp gmp_build)
add_dependencies(omnibyte_gmpxx gmp_build)

# --- Adapter library ---
add_library(omnibyte_gmp_adapter STATIC GMP.cpp GMPAdapter.cpp)

target_include_directories(omnibyte_gmp_adapter PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..       # exposes <GMP/...>
    ${CMAKE_CURRENT_SOURCE_DIR}/../..    # exposes <common/GMP/...>
)
target_link_libraries(omnibyte_gmp_adapter
    PUBLIC  omnibyte_gmp
            omnibyte_gmpxx
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
