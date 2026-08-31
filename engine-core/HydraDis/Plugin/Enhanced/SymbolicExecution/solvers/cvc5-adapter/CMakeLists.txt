# solvers/cvc5-adapter — placeholder adapter for: cvc5
#
# Stage 1 scaffolding only. Real backend wiring is commented out below — enable
# when the library is available.
cmake_minimum_required(VERSION 3.22.1)

project(solvers_cvc5 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(solvers_cvc5 STATIC solvers_cvc5.cpp)

# ===== PLACEHOLDER — real backend wiring (uncomment when cvc5 is available) =====
# find_package(PkgConfig REQUIRED)
# pkg_check_modules(solvers_cvc5_PC REQUIRED cvc5)
# find_package(Z3 REQUIRED)   # if applicable
# target_include_directories(solvers_cvc5 PUBLIC ...)
# target_link_libraries(solvers_cvc5 PUBLIC ...)
