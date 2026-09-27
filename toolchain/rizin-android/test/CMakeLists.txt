# Minimal link test for the rizin Android prebuilt.
# Build with the NDK toolchain file so ANDROID_ABI is set:
#   cmake -S . -B <build> \
#     -DCMAKE_TOOLCHAIN_FILE=$NDK/build/cmake/android.toolchain.cmake \
#     -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-24
cmake_minimum_required(VERSION 3.22)
project(rizin_link_test C)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/..")
find_package(RizinAndroid REQUIRED)

add_executable(test_rz_asm test_rz_asm.c)
target_link_libraries(test_rz_asm PRIVATE RizinAndroid::Rizin)
