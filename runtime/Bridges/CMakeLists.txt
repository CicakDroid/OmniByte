# runtime/Bridges — JNI / root-service / module bridges.
cmake_minimum_required(VERSION 3.22.1)

project(runtime_bridges LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(JavaNativeBridge)
add_subdirectory(RootServiceBridge)
add_subdirectory(ModuleBridge)
