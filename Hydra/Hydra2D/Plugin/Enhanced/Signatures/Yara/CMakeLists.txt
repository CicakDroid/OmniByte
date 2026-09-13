cmake_minimum_required(VERSION 3.14)

option(ENABLE_YARA "Enable YARA rule scanning (requires libyara)" ON)

if(NOT ENABLE_YARA)
    # Minimal stub for linking
    add_library(yara_stub STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/stub/yara_stub.cpp
    )
    target_include_directories(yara_stub PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
    return()
endif()

# Try to find system libyara first
find_package(yara QUIET)

if(NOT yara_FOUND)
    # Bundle libyara as static library
    include(FetchContent)
    FetchContent_Declare(
        libyara
        GIT_REPOSITORY https://github.com/VirusTotal/libyara.git
        GIT_TAG        yara-4.5.0
        GIT_SHALLOW    TRUE
    )
    
    # Build libyara with minimal features for Android
    set(YARA_BUILD_COMMAND_LINE OFF CACHE BOOL "" FORCE)
    set(YARA_BUILD_MODULE OFF CACHE BOOL "" FORCE)
    set(YARA_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    
    FetchContent_MakeAvailable(libyara)
    set(yara_LIBRARIES libyara)
    set(yara_INCLUDE_DIRS ${libyara_SOURCE_DIR} ${libyara_BINARY_DIR})
endif()

add_library(yara STATIC
    YaraEngine.cpp
    YaraUpdater.cpp
    YaraRulesManager.cpp
    plugin_yara.cpp
)

target_include_directories(yara PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${yara_INCLUDE_DIRS}
)

target_link_libraries(yara PRIVATE ${yara_LIBRARIES})
target_compile_definitions(yara PRIVATE HAS_YARA)

# Install rules directory
install(DIRECTORY rules/ DESTINATION share/omnibyte/yara/rules)
