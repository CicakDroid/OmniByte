cmake_minimum_required(VERSION 3.18)

# updater library — auto-update support (GitHub releases, download, install)
set(UPDATER_SOURCES
    GithubReleaseChecker/GithubReleaseChecker.cpp
    Downloader/Downloader.cpp
    Installer/Installer.cpp
)

set(UPDATER_HEADERS
    GithubReleaseChecker/GithubReleaseChecker.h
    Downloader/Downloader.h
    Installer/Installer.h
)

add_library(updater STATIC ${UPDATER_SOURCES} ${UPDATER_HEADERS})

target_include_directories(updater PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

find_package(CURL REQUIRED)
find_package(OpenSSL REQUIRED)

target_link_libraries(updater PUBLIC
    CURL::libcurl
    OpenSSL::SSL
)

set_target_properties(updater PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)
