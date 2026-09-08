# runtime/Bridges — JNI bridges (FreedomService + Module).
cmake_minimum_required(VERSION 3.22.1)

project(runtime_bridges LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(FreedomServiceBridge)
add_subdirectory(ModuleBridge)

add_library(bridge_loader SHARED BridgeLoader.cpp)
target_link_libraries(bridge_loader PRIVATE freedom_service_bridge module_bridge)
target_include_directories(bridge_loader PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/FreedomServiceBridge
    ${CMAKE_CURRENT_SOURCE_DIR}/ModuleBridge
    ${CMAKE_CURRENT_SOURCE_DIR}
)
