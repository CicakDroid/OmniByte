#!/bin/bash
# build-native.sh — Build OmniByte native libraries (C++17)
# Supports Android SDK & NDK build with configurable ABI, API level, and more

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
APP_CPP_DIR="$PROJECT_DIR/app/src/main/cpp"
BUILD_DIR="$APP_CPP_DIR/build"

# Defaults
ABI="arm64-v8a"
API_LEVEL=23
BUILD_TYPE="Release"
CLEAN=0
VERBOSE=0
PARALLEL_JOBS=$(nproc 2>/dev/null || echo 4)

usage() {
    cat <<EOF
Usage: $(basename "$0") [OPTIONS]

Build OmniByte native libraries for Android.

Options:
  --abi <abi>          Target ABI (default: arm64-v8a)
                         Supported: arm64-v8a, armeabi-v7a, x86_64, x86
  --api <level>        Android API level (default: 23)
  --build-type <type>  Build type: Release, Debug (default: Release)
  --ndk <path>         Path to Android NDK (overrides \$ANDROID_NDK)
  --sdk <path>         Path to Android SDK (overrides \$ANDROID_SDK_ROOT)
  --clean              Clean build directory before building
  --jobs <n>           Parallel build jobs (default: $(nproc 2>/dev/null || echo 4))
  --verbose            Verbose make output
  -h, --help           Show this help message

Examples:
  $(basename "$0")
  $(basename "$0") --abi armeabi-v7a --api 21
  $(basename "$0") --abi arm64-v8a --clean --verbose
  $(basename "$0") --ndk /opt/android-nddk --abi arm64-v8a
EOF
    exit 0
}

# Parse arguments
while [[ $# -gt 0 ]]; do
    case "$1" in
        --abi)        ABI="$2"; shift 2 ;;
        --api)        API_LEVEL="$2"; shift 2 ;;
        --build-type) BUILD_TYPE="$2"; shift 2 ;;
        --ndk)        ANDROID_NDK="$2"; shift 2 ;;
        --sdk)        ANDROID_SDK_ROOT="$2"; shift 2 ;;
        --clean)      CLEAN=1; shift ;;
        --jobs)       PARALLEL_JOBS="$2"; shift 2 ;;
        --verbose)    VERBOSE=1; shift ;;
        -h|--help)    usage ;;
        *)            echo "Unknown option: $1"; usage ;;
    esac
done

# Resolve NDK
if [[ -z "$ANDROID_NDK" ]]; then
    # Try common locations
    for ndk in \
        "$ANDROID_SDK_ROOT/ndk/"* \
        "$HOME/Android/Sdk/ndk/"* \
        "/opt/android-ndk" \
        "/usr/local/android-ndk"; do
        if [[ -d "$ndk" ]] && [[ -f "$ndk/build/cmake/android.toolchain.cmake" ]]; then
            ANDROID_NDK="$ndk"
            break
        fi
    done
fi

if [[ -z "$ANDROID_NDK" ]] || [[ ! -f "$ANDROID_NDK/build/cmake/android.toolchain.cmake" ]]; then
    echo "Error: Android NDK not found."
    echo ""
    echo "Set ANDROID_NDK environment variable or pass --ndk <path>."
    echo "Download NDK: https://developer.android.com/ndk/downloads"
    exit 1
fi

TOOLCHAIN_FILE="$ANDROID_NDK/build/cmake/android.toolchain.cmake"

echo "=== OmniByte Native Build ==="
echo "  ABI:        $ABI"
echo "  API Level:  android-$API_LEVEL"
echo "  Build Type: $BUILD_TYPE"
echo "  NDK:        $ANDROID_NDK"
echo "  Jobs:       $PARALLEL_JOBS"
echo ""

# Clean if requested
if [[ $CLEAN -eq 1 ]]; then
    echo "Cleaning build directory..."
    rm -rf "$BUILD_DIR"
fi

# Create build directory
mkdir -p "$BUILD_DIR"

# CMake flags
CMAKE_ARGS=(
    -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE"
    -DANDROID_ABI="$ABI"
    -DANDROID_PLATFORM="android-$API_LEVEL"
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
    -DANDROID_STL=c++_shared
)

VERBOSE_FLAG=""
if [[ $VERBOSE -eq 1 ]]; then
    VERBOSE_FLAG="VERBOSE=1"
fi

echo "Configuring CMake..."
cd "$BUILD_DIR"
cmake "${CMAKE_ARGS[@]}" "$APP_CPP_DIR"

echo ""
echo "Building..."
cmake --build . --parallel "$PARALLEL_JOBS" $VERBOSE_FLAG

echo ""
echo "=== Build complete ==="
echo "Output: $BUILD_DIR"
