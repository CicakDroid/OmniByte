# SymbolicExecution/engine — execution engines (Triton).
cmake_minimum_required(VERSION 3.22.1)

project(enhanced_symbolicexec_engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# triton-adapter: Triton's find_package(Z3) needs a system Z3 install; the
# in-tree solvers/z3-adapter only defines a CMake target, not a Z3 package.
# Not linked into engine_core yet — re-enable after Z3 is on CMAKE_PREFIX_PATH.
# add_subdirectory(triton-adapter)
