# Enhanced plugins — heavy analysis features, all opt-in (never auto-run).
cmake_minimum_required(VERSION 3.22.1)

project(hydradis_plugin_enhanced LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(AST)
add_subdirectory(CFG)
add_subdirectory(FunctionResolver)
add_subdirectory(RTTI)
add_subdirectory(FindCrypt)
add_subdirectory(Emulation)
add_subdirectory(SymbolicExecution)
