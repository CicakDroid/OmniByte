cmake_minimum_required(VERSION 3.22.1)
project(ApkSignatureBypass LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Sub-projects ---
add_subdirectory(ApkSigKiller)
add_subdirectory(ApkSigKillerEx)

# --- Orchestrator ---
add_library(ApkSignatureBypass STATIC
    ApkSignatureBypass.cpp
    SignatureExtractor.cpp
    SignatureInjector.cpp
    LoadedApkSpoofer.cpp
    NativePmsHook.cpp
)

target_include_directories(ApkSignatureBypass PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ApkSignatureBypass PRIVATE
    ApkSigKillerAdapter
    ApkSigKillerExAdapter
    dl
)

target_compile_definitions(ApkSignatureBypass PRIVATE
    APKSIGNATUREBYPASS_VERSION="1.0.0"
)
