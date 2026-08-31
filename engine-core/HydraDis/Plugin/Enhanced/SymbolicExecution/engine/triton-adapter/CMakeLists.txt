# engine/triton-adapter — placeholder adapter for: triton
#
# Stage 1 scaffolding only. Real backend wiring is commented out below — enable
# when the library is available.
cmake_minimum_required(VERSION 3.22.1)

project(symbolicexec_triton LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(symbolicexec_triton STATIC symbolicexec_triton.cpp)

# ===== PLACEHOLDER — real backend wiring (uncomment when triton is available) =====
# find_package(PkgConfig REQUIRED)
# pkg_check_modules(symbolicexec_triton_PC REQUIRED triton)
# find_package(Z3 REQUIRED)   # if applicable
# target_include_directories(symbolicexec_triton PUBLIC ...)
# target_link_libraries(symbolicexec_triton PUBLIC ...)
