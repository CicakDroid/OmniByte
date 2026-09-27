cmake_minimum_required(VERSION 3.22.1)

project(disassembler_protobuf LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    protobuf
    GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
    GIT_TAG        v25.3
    GIT_SHALLOW    TRUE
)

# Lib saja -- tidak perlu protoc, test, shared lib, maupun zlib.
set(protobuf_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(protobuf_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(protobuf_BUILD_PROTOC_BINARIES OFF CACHE BOOL "" FORCE)
set(protobuf_WITH_ZLIB OFF CACHE BOOL "" FORCE)
set(protobuf_BUILD_LIBUPB OFF CACHE BOOL "" FORCE)
# Abseil di-fetch oleh protobuf sendiri (module) -- repo ini sudah memvendored
# Abseil terpisah, jadi find_package tidak akan menemukannya di mesin build.
set(protobuf_ABSL_PROVIDER "module" CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(protobuf)

add_library(disassembler_protobuf STATIC ProtobufAdapter.cpp)

target_link_libraries(disassembler_protobuf PUBLIC libprotobuf)

# Root repo, bukan hanya folder backend -- include ProtobufAdapter.h memakai
# pola repo-root-relative ("Disassembler/...").
target_include_directories(disassembler_protobuf PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
)
