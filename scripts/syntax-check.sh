#!/bin/bash
# syntax-check.sh — NDK syntax check using compile_commands.json
# Reads include paths from compile_commands.json instead of hardcoding.
#
# Usage:
#   ./scripts/syntax-check.sh <file.cpp>           # Check single file
#   ./scripts/syntax-check.sh --all                 # Check all Hydra2D files
#   ./scripts/syntax-check.sh --configure           # Run cmake configure first
#   ./scripts/syntax-check.sh --fetch-boost         # Download Boost 1.92.0 locally
#
# Prerequisite: cmake configure must be run at least once to generate
# compile_commands.json and fetch Boost headers.
#
# Build note: Hydra2D/CMakeLists.txt has CMAKE_EXPORT_COMPILE_COMMANDS ON.
#
# Boost offline strategy:
#   1. ./scripts/syntax-check.sh --fetch-boost      # Download once (needs network)
#   2. ./scripts/syntax-check.sh --configure         # Uses cached Boost (no network)
#   3. ./scripts/syntax-check.sh <file>              # Check with compile_commands.json

set -euo pipefail

NDK_PATH="/opt/android-ndk-r29"
TOOLCHAIN="${NDK_PATH}/toolchains/llvm/prebuilt/linux-x86_64"
CC="${TOOLCHAIN}/aarch64-linux-android23-clang++"
BUILD_DIR="build/ndk-check"
COMPILE_DB="${BUILD_DIR}/compile_commands.json"
BOOST_CACHE="${HOME}/.cache/omnibyte/boost-1.92.0-src"
BOOST_URL="https://github.com/boostorg/boost/releases/download/boost-1.92.0/boost-1.92.0-b2-nodocs.tar.gz"

cd "$(dirname "$0")/.."

# --- Helpers ---

check_file() {
    local file="$1"
    if [ ! -f "$file" ]; then
        echo "ERROR: File not found: $file"
        return 1
    fi

    if [ ! -f "$COMPILE_DB" ]; then
        echo "ERROR: compile_commands.json not found at ${COMPILE_DB}"
        echo "Run: ./scripts/syntax-check.sh --configure"
        return 1
    fi

    # Extract -I flags from compile_commands.json for this file
    local includes
    includes=$(python3 -c "
import json, sys, os

db = json.load(open('${COMPILE_DB}'))
target = os.path.abspath('${file}')

# Find matching entry
entry = None
for e in db:
    if os.path.abspath(e.get('file', '')) == target:
        entry = e
        break

if not entry:
    # Fallback: find any entry from same directory
    target_dir = os.path.dirname(target)
    for e in db:
        if os.path.dirname(os.path.abspath(e.get('file', ''))) == target_dir:
            entry = e
            break

if not entry:
    # Last resort: use any entry that has boost include
    for e in db:
        if 'boost' in e.get('command', '').lower():
            entry = e
            break

if entry:
    args = entry.get('command', '').split()
    flags = [a for a in args if a.startswith('-I') or a.startswith('-std') or a.startswith('-D')]
    print(' '.join(flags))
else:
    # Minimal fallback
    print('-std=c++17 -I. -Icommon')
" 2>/dev/null)

    if [ -z "$includes" ]; then
        echo "WARNING: Could not extract flags from compile_commands.json for ${file}"
        echo "  Using minimal fallback flags"
        includes="-std=c++17 -I. -Icommon -IHydra -IHydra/Hydra2D"
    fi

    # Run syntax check
    local output
    if output=$($CC -std=c++17 -fsyntax-only $includes "$file" 2>&1); then
        echo "OK    ${file}"
        return 0
    else
        echo "FAIL  ${file}"
        echo "$output" | sed 's/^/      /'
        return 1
    fi
}

fetch_boost() {
    echo "=== Fetching Boost 1.92.0 to local cache ==="
    echo "  Cache: ${BOOST_CACHE}"
    echo ""

    if [ -d "${BOOST_CACHE}" ]; then
        echo "Boost already cached at ${BOOST_CACHE}"
        echo "  To re-download, remove: rm -rf ${BOOST_CACHE}"
        return 0
    fi

    mkdir -p "$(dirname "${BOOST_CACHE}")"

    echo "Downloading ${BOOST_URL}..."
    local tmp_tar="/tmp/boost-1.92.0.tar.gz"

    if command -v curl &>/dev/null; then
        curl -L -o "${tmp_tar}" "${BOOST_URL}"
    elif command -v wget &>/dev/null; then
        wget -O "${tmp_tar}" "${BOOST_URL}"
    else
        echo "ERROR: Neither curl nor wget found"
        return 1
    fi

    echo "Extracting..."
    tar -xzf "${tmp_tar}" -C "$(dirname "${BOOST_CACHE}")"

    # The tarball extracts to boost-1.92.0/ directory
    local extracted="$(dirname "${BOOST_CACHE}")/boost-1.92.0"
    if [ -d "${extracted}" ]; then
        mv "${extracted}" "${BOOST_CACHE}"
    fi

    rm -f "${tmp_tar}"
    echo "SUCCESS: Boost cached at ${BOOST_CACHE}"
}

configure() {
    echo "Running cmake configure to generate compile_commands.json..."
    echo ""

    mkdir -p "$BUILD_DIR"

    # Build cmake args
    local CMAKE_ARGS=(-B "$BUILD_DIR")

    # Use NDK toolchain if available
    local TOOLCHAIN_FILE="${NDK_PATH}/build/cmake/android.toolchain.cmake"
    if [ -f "$TOOLCHAIN_FILE" ]; then
        CMAKE_ARGS+=(
            -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE"
            -DANDROID_ABI=arm64-v8a
            -DANDROID_PLATFORM=android-23
        )
    else
        echo "WARNING: NDK toolchain not found at ${TOOLCHAIN_FILE}"
        echo "  Falling back to host cmake configure"
    fi

    # Use cached Boost if available (skip network)
    if [ -d "${BOOST_CACHE}" ]; then
        echo "Using cached Boost: ${BOOST_CACHE}"
        CMAKE_ARGS+=(-DFETCHCONTENT_SOURCE_DIR_BOOST="${BOOST_CACHE}")
    else
        echo "Boost not cached. Will download via FetchContent."
        echo "  Run './scripts/syntax-check.sh --fetch-boost' first for offline mode."
    fi

    # Configure Hydra2D
    if cmake "${CMAKE_ARGS[@]}" Hydra/Hydra2D 2>&1; then
        echo ""
        echo "SUCCESS: compile_commands.json generated at ${COMPILE_DB}"

        # Check Boost availability
        local boost_dir="${BUILD_DIR}/_deps/boost-src"
        if [ -d "${boost_dir}" ]; then
            echo "  Boost headers: ${boost_dir}/"
        else
            echo "  WARNING: Boost headers not found in build dir"
        fi
    else
        echo ""
        echo "ERROR: cmake configure failed"
        return 1
    fi
}

# --- Main ---

if [ $# -eq 0 ]; then
    echo "Usage:"
    echo "  $0 <file.cpp>           # Check single file"
    echo "  $0 --all                # Check all Hydra2D .cpp files"
    echo "  $0 --configure          # Run cmake configure first"
    echo "  $0 --fetch-boost        # Download Boost 1.92.0 locally"
    exit 1
fi

case "$1" in
    --configure|-c)
        configure
        ;;
    --fetch-boost|-f)
        fetch_boost
        ;;
    --all|-a)
        echo "=== NDK Syntax Check — All Hydra2D Files ==="
        echo ""

        # Check compile_commands.json exists
        if [ ! -f "$COMPILE_DB" ]; then
            echo "compile_commands.json not found. Running configure first..."
            configure
            echo ""
        fi

        echo "Using compile_commands.json: ${COMPILE_DB}"
        echo ""

        FILES=$(find Hydra/Hydra2D -name '*.cpp' \
            -not -path '*/backends/*' \
            -not -path '*/ScriptHooks/*' \
            | sort)

        total=0
        passed=0
        failed=0

        for f in $FILES; do
            total=$((total + 1))
            if check_file "$f"; then
                passed=$((passed + 1))
            else
                failed=$((failed + 1))
            fi
        done

        echo ""
        echo "=== Results: ${passed}/${total} passed, ${failed} failed ==="
        [ $failed -eq 0 ]
        ;;
    *)
        check_file "$1"
        ;;
esac
