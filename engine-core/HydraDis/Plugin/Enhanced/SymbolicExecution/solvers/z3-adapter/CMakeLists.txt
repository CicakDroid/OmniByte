# solvers/z3-adapter — placeholder adapter for: z3
#
# Stage 1 scaffolding only. Real backend wiring is commented out below — enable
# when the library is available.
cmake_minimum_required(VERSION 3.22.1)

project(solvers_z3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(solvers_z3 STATIC solvers_z3.cpp)

# ===== PLACEHOLDER — real backend wiring (uncomment when z3 is available) =====
# find_package(PkgConfig REQUIRED)
# pkg_check_modules(solvers_z3_PC REQUIRED z3)
# find_package(Z3 REQUIRED)   # if applicable
# target_include_directories(solvers_z3 PUBLIC ...)
# target_link_libraries(solvers_z3 PUBLIC ...)
