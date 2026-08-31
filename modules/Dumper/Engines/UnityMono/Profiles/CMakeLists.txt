# UnityMono/Profiles — version profile headers + factory.
cmake_minimum_required(VERSION 3.22.1)

project(unitymono_profiles LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(unitymono_profiles STATIC unitymono_profiles.cpp)
target_include_directories(unitymono_profiles PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
