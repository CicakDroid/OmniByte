cmake_minimum_required(VERSION 3.22.1)

project(solvers_cvc5 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    cvc5
    GIT_REPOSITORY https://github.com/cvc5/cvc5.git
    GIT_TAG        cvc5-1.3.4
    GIT_SHALLOW    TRUE
)

set(CVC5_BUILD_PYTHON_BINDINGS OFF CACHE BOOL "" FORCE)
set(CVC5_BUILD_JAVA_BINDINGS OFF CACHE BOOL "" FORCE)
set(CVC5_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(CVC5_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(CVC5_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

# Point FindGMP at the prebuilt aarch64 GMP (same tree as common/GMP).
set(GMP_INCLUDE_DIR /tmp/gmp-install-arm64-v8a/include CACHE PATH "" FORCE)
set(GMP_LIBRARIES /tmp/gmp-install-arm64-v8a/lib/libgmp.a CACHE FILEPATH "" FORCE)
set(GMPXX_INCLUDE_DIR /tmp/gmp-install-arm64-v8a/include CACHE PATH "" FORCE)
set(GMPXX_LIBRARIES /tmp/gmp-install-arm64-v8a/lib/libgmpxx.a CACHE FILEPATH "" FORCE)

# cvc5 only accepts Production/Debug/Testing/Competition — map Release.
set(_cvc5_saved_bt ${CMAKE_BUILD_TYPE})
set(CMAKE_BUILD_TYPE Production)
FetchContent_MakeAvailable(cvc5)
set(CMAKE_BUILD_TYPE ${_cvc5_saved_bt})

add_library(solvers_cvc5 STATIC solvers_cvc5.cpp)

target_link_libraries(solvers_cvc5 PUBLIC cvc5)

target_include_directories(solvers_cvc5 PUBLIC
    ${cvc5_SOURCE_DIR}/include
    ${cvc5_BINARY_DIR}
)
