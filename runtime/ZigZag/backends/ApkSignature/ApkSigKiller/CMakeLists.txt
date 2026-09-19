cmake_minimum_required(VERSION 3.22.1)
project(ApkSigKillerAdapter LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Fetch upstream ApkSignatureKiller ---
include(FetchContent)

FetchContent_Declare(
    ApkSignatureKiller
    GIT_REPOSITORY https://github.com/L-JINBIN/ApkSignatureKiller.git
    GIT_TAG        master
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
)

# Don't build upstream — we only need the source for smali injection.
set(SKIP_ApkSignatureKiller_BUILD TRUE)
FetchContent_MakeAvailable(ApkSignatureKiller)

# --- Sources ---
add_library(ApkSigKillerAdapter STATIC
    ApkSigKillerAdapter.cpp
)

target_include_directories(ApkSigKillerAdapter PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link against upstream sources (for smali templates).
if(Apksignaturekiller_SOURCE_DIR)
    target_include_directories(ApkSigKillerAdapter PRIVATE
        ${Apksignaturekiller_SOURCE_DIR}/src
        ${Apksignaturekiller_SOURCE_DIR}/hook
    )
    # Copy upstream smali templates into build dir for runtime use.
    file(GLOB SMALI_TEMPLATES "${Apksignaturekiller_SOURCE_DIR}/hook/*.smali")
    foreach(SMALI ${SMALI_TEMPLATES})
        get_filename_component(SMALI_NAME ${SMALI} NAME)
        configure_file(${SMALI} ${CMAKE_CURRENT_BINARY_DIR}/${SMALI_NAME} COPYONLY)
    endforeach()
endif()

target_link_libraries(ApkSigKillerAdapter PRIVATE
    ApkSignatureBypass
    dl
)

target_compile_definitions(ApkSigKillerAdapter PRIVATE
    APKSIGKILLER_VERSION="1.0.0"
    APKSIGKILLER_UPSTREAM="L-JINBIN/ApkSignatureKiller"
)
