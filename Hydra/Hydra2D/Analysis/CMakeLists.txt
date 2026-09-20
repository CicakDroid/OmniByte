# Hydra/Hydra2D/Analysis — Binary analysis algorithms.
# Implements IAnalysis interface with proven algorithms from research doc:
#   Structure, Stack, List, Tree, Packers, Obfuscate, Taint
cmake_minimum_required(VERSION 3.22.1)
project(hydradis_analysis LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Sources
add_library(hydradis_analysis STATIC
    Structure.cpp
    Stack.cpp
    List.cpp
    Tree.cpp
    Packers.cpp
    Obfuscate.cpp
    Taint.cpp
    Functions.cpp
    Variables.cpp
    Parameters.cpp
    Types.cpp
    Confidences.cpp
    Strings.cpp
)

# Public headers
target_include_directories(hydradis_analysis PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link common dependencies
target_link_libraries(hydradis_analysis PUBLIC
    omnibyte_boost
    omnibyte_gmp
    omnibyte_taskflow
)
