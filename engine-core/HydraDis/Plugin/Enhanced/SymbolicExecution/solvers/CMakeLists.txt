# SymbolicExecution/solvers — interchangeable SMT solvers (ISolverBackend contract).
cmake_minimum_required(VERSION 3.22.1)

project(enhanced_symbolicexec_solvers LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(z3-adapter)
add_subdirectory(cvc5-adapter)
