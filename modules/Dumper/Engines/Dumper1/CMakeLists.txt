# Dumper Engine: Dumper1 — adapted from il2cpp-dumper-rs.
# Analyzer (metadata parsing + XOR decryption) + Resolver (CR/MR finder) + Profiles.
cmake_minimum_required(VERSION 3.22.1)

project(engine_dumper1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(Analyzer)
add_subdirectory(Resolver)
add_subdirectory(Output)
add_subdirectory(Profiles)
