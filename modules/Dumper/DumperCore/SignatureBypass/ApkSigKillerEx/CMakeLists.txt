# ApkSignatureKillerEx — Extended APK signature bypass.
cmake_minimum_required(VERSION 3.22.1)

project(apk_sig_killer_ex LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(apk_sig_killer_ex STATIC ApkSigKillerEx.cpp)
