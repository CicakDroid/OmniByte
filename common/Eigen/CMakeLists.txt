# Eigen 5.0.0 (header-only linear algebra) — shared, not wired to Hydra.
# FetchContent pattern mirrors common/Boost/CMakeLists.txt.
include(FetchContent)
cmake_policy(SET CMP0135 NEW)

set(OMNIBYTE_EIGEN_VERSION 5.0.0)
set(OMNIBYTE_EIGEN_SHA256 315c881e19e17542a7d428c5aa37d113c89b9500d350c433797b730cd449c056)

FetchContent_Declare(
    eigen
    URL https://gitlab.com/libeigen/eigen/-/archive/${OMNIBYTE_EIGEN_VERSION}/eigen-${OMNIBYTE_EIGEN_VERSION}.tar.gz
    URL_HASH SHA256=${OMNIBYTE_EIGEN_SHA256}
    SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/eigen5
)
FetchContent_MakeAvailable(eigen)

# Headers land at <bin>/eigen5/Eigen/*.  Expose <bin> so consumers write
#   #include <eigen5/Eigen/Dense>
# exactly as spec'd.  STATIC (not INTERFACE) so Eigen.cpp compiles once.
add_library(omnibyte_eigen STATIC Eigen.cpp)
target_include_directories(omnibyte_eigen PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}      # <bin>/eigen5/... -> <eigen5/Eigen/Dense>
    ${CMAKE_CURRENT_SOURCE_DIR}/..   # common/          -> <Eigen/Eigen.h>
)
add_library(OmniByte::Eigen ALIAS omnibyte_eigen)
