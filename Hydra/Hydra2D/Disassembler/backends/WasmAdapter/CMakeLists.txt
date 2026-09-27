cmake_minimum_required(VERSION 3.22.1)

project(disassembler_wasm LANGUAGES C CXX)

# wabt menuntut C++20 (std::span di binary-reader.h). Target wabt sendiri
# sudah propagate cxx_std_20, tapi diset eksplisit supaya WasmAdapter.cpp
# ikut dikompilasi C++20 tanpa bergantung pada urutan link.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    wabt
    GIT_REPOSITORY https://github.com/WebAssembly/wabt.git
    GIT_TAG        02418f72a70466b45805891843f114c7f602eb34
    # SHA penuh tidak bisa diambil dengan shallow clone.
    GIT_SHALLOW    FALSE
)

# Lib decoder saja: tanpa test, tool (wat2wasm dll.), fuzz, libwasm, WASI,
# maupun aturan install -- sama seperti backend lain yang hanya butuh .a.
set(BUILD_TESTS          OFF CACHE BOOL "" FORCE)
set(BUILD_TOOLS          OFF CACHE BOOL "" FORCE)
set(BUILD_FUZZ_TOOLS     OFF CACHE BOOL "" FORCE)
set(BUILD_LIBWASM        OFF CACHE BOOL "" FORCE)
set(WITH_WASI            OFF CACHE BOOL "" FORCE)
set(WABT_INSTALL_RULES   OFF CACHE BOOL "" FORCE)
# SHA-256 internal (picosha2) supaya tidak menarik OpenSSL -- toolchain
# Android NDK tidak menyediakannya.
set(USE_INTERNAL_SHA256  ON  CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(wabt)

add_library(disassembler_wasm STATIC WasmAdapter.cpp)

target_link_libraries(disassembler_wasm PUBLIC wabt::wabt)

# Root repo, bukan hanya folder backend -- include WasmAdapter.h memakai
# pola repo-root-relative ("Disassembler/...").
target_include_directories(disassembler_wasm PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
)
