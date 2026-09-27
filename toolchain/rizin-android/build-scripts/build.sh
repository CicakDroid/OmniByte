#!/usr/bin/env bash
# Cross-build static rizin (librz) for Android ABIs.
# Output: prebuilt/<abi>/{lib,include}
# Env overrides:
#   NDK       - Android NDK path            (default: /opt/android-ndk-r29)
#   API       - minSdk API level            (default: 24)
#   RIZIN_TAG - rizin git tag to build      (default: v0.9.1)
#   ABIS      - space-separated ABI list    (default: "arm64-v8a armeabi-v7a")
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
NDK="${NDK:-/opt/android-ndk-r29}"
API="${API:-24}"
RIZIN_TAG="${RIZIN_TAG:-v0.9.1}"
read -r -a ABIS <<< "${ABIS:-arm64-v8a armeabi-v7a}"

case "$(uname -m)" in
  aarch64) HOST_TAG=linux-aarch64 ;;
  x86_64)  HOST_TAG=linux-x86_64 ;;
  *) echo "unsupported host arch: $(uname -m)" >&2; exit 1 ;;
esac
TOOLCHAIN="$NDK/toolchains/llvm/prebuilt/$HOST_TAG"
[[ -x "$TOOLCHAIN/bin/clang" ]] || { echo "NDK toolchain not found: $TOOLCHAIN" >&2; exit 1; }

# Host pkg-config must not resolve host libraries into the cross build;
# every dependency comes from rizin's bundled subproject wraps instead.
export PKG_CONFIG_LIBDIR=/nonexistent

SRC="$ROOT/.build/rizin-$RIZIN_TAG"
if [[ ! -d "$SRC/.git" ]]; then
  echo "== cloning rizin $RIZIN_TAG =="
  git clone --depth 1 --branch "$RIZIN_TAG" https://github.com/rizinorg/rizin.git "$SRC"
fi

for ABI in "${ABIS[@]}"; do
  case "$ABI" in
    arm64-v8a)   TEMPLATE=cross-files/android-arm64.txt ;;
    armeabi-v7a) TEMPLATE=cross-files/android-armv7.txt ;;
    *) echo "unknown ABI: $ABI" >&2; exit 1 ;;
  esac
  BUILD="$ROOT/.build/$ABI"
  PREFIX="$ROOT/prebuilt/$ABI"
  CROSS="$BUILD/cross.txt"
  mkdir -p "$BUILD"
  sed -e "s|TOOLCHAIN|$TOOLCHAIN|g" -e "s|API|$API|g" \
      "$ROOT/$TEMPLATE" > "$CROSS"

  RECONFIGURE=""
  [[ -f "$BUILD/meson-private/coredata.dat" ]] && RECONFIGURE="--reconfigure"

  echo "== meson setup: $ABI =="
  meson setup "$BUILD" "$SRC" \
    --cross-file "$CROSS" \
    --prefix "$PREFIX" \
    --buildtype release \
    --default-library static \
    -Dcli=disabled \
    -Ddebugger=false \
    -Dstatic_runtime=true \
    -Denable_tests=false \
    -Denable_rz_test=false \
    -Denable_benchmarks=false \
    -Denable_examples=false \
    $RECONFIGURE

  echo "== compile: $ABI =="
  # -j 4: default nproc parallelism OOM-killed ninja on low-RAM hosts; raise via JOBS=
  meson compile -C "$BUILD" -j "${JOBS:-4}"
  echo "== install: $ABI =="
  meson install -C "$BUILD"
  echo "== done: $ABI -> $PREFIX =="
done
