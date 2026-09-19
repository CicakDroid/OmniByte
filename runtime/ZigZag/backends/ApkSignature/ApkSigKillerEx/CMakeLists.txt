cmake_minimum_required(VERSION 3.22.1)
project(ApkSigKillerExAdapter LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Fetch upstream ApkSignatureKillerEx ---
include(FetchContent)

FetchContent_Declare(
    ApkSignatureKillerEx
    GIT_REPOSITORY https://github.com/L-JINBIN/ApkSignatureKillerEx.git
    GIT_TAG        main
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
)

set(SKIP_ApkSignatureKillerEx_BUILD TRUE)
FetchContent_MakeAvailable(ApkSignatureKillerEx)

# --- Sources ---
add_library(ApkSigKillerExAdapter STATIC
    ApkSigKillerAdapter.cpp
)

target_include_directories(ApkSigKillerExAdapter PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link against upstream sources.
if(Apksignaturekillerex_SOURCE_DIR)
    target_include_directories(ApkSigKillerExAdapter PRIVATE
        ${Apksignaturekillerex_SOURCE_DIR}/killer/src/main/java
    )
    # Copy upstream smali templates.
    file(GLOB_RECURSE SMALI_TEMPLATES "${Apksignaturekillerex_SOURCE_DIR}/*.smali")
    foreach(SMALI ${SMALI_TEMPLATES})
        get_filename_component(SMALI_NAME ${SMALI} NAME)
        configure_file(${SMALI} ${CMAKE_CURRENT_BINARY_DIR}/${SMALI_NAME} COPYONLY)
    endforeach()
endif()

target_link_libraries(ApkSigKillerExAdapter PRIVATE
    ApkSignatureBypass
    dl
)

target_compile_definitions(ApkSigKillerExAdapter PRIVATE
    APKSIGKILLEREX_VERSION="1.0.0"
    APKSIGKILLEREX_UPSTREAM="L-JINBIN/ApkSignatureKillerEx"
)
