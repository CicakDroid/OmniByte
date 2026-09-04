# DeCrypt3 — Decrypt algorithms detected by FindCrypt3.
# Source: FIPS 197 (AES), FIPS 46-3 (DES), RFC 7914 (ChaCha20),
#         RFC 6229 (RC4), Wheeler & Needham (TEA/XTEA/XXTEA).
cmake_minimum_required(VERSION 3.22.1)

project(decrypt3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(decrypt3 STATIC DeCrypt3.cpp)
