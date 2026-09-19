cmake_minimum_required(VERSION 3.22.1)
project(omnibyte_json LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(NLOHMANN_JSON_VERSION "3.11.3")

include(FetchContent)
cmake_policy(SET CMP0135 NEW)

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v${NLOHMANN_JSON_VERSION}
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(nlohmann_json)

add_library(omnibyte_json STATIC
    Json.cpp
    JsonAdapter.cpp
)

target_include_directories(omnibyte_json PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..   # exposes <common/Deserialization-Serialization/Json/...>
    ${CMAKE_CURRENT_SOURCE_DIR}/../.. # exposes <common/...>
)

target_link_libraries(omnibyte_json PUBLIC nlohmann_json::nlohmann_json PRIVATE log)
