#!/usr/bin/env bash
# build-rizin-android.sh
# Cross-compile Rizin jadi single static blob binary untuk Android arm64-v8a
# Dipakai sebagai backend engine-core/HydraDis/Disassembler/backends/rizin-adapter
#
# Jalankan dari mesin build (Termux native / Linux host), BUKAN di dalam proot-distro
# kalau NDK-nya dipasang di sisi Termux native.
#
# Prasyarat:
#   pkg install python git ninja        # di Termux native
#   pip install meson --break-system-packages
#   NDK sudah terpasang, path-nya diisi ke variabel NDK di bawah

set -euo pipefail

# ── Konfigurasi ──────────────────────────────────────────────
NDK="${NDK:-$HOME/android-ndk-r27}"
API="${API:-24}"
HOST_TAG="${HOST_TAG:-linux-x86_64}"
TOOLCHAIN="$NDK/toolchains/llvm/prebuilt/$HOST_TAG"
PREFIX="$HOME/omnibyte-toolchain/rizin-android-arm64"
CROSS_FILE="$HOME/OmniByte/toolchain/rizin-android/android-arm64.ini"
SRC_DIR="$HOME/rizin"
# ─────────────────────────────────────────────────────────────

if [ ! -d "$TOOLCHAIN" ]; then
  echo "NDK toolchain tidak ditemukan di: $TOOLCHAIN"
  echo "Cek path NDK atau set HOST_TAG yang benar (linux-x86_64 / darwin-x86_64)."
  exit 1
fi

# 1. Generate cross-file dari template dengan path NDK yang sudah di-resolve
mkdir -p "$(dirname "$CROSS_FILE")"
sed \
  -e "s|TOOLCHAIN|$TOOLCHAIN|g" \
  -e "s|API|$API|g" \
  android-arm64.ini > "$CROSS_FILE"
echo "[1/4] Cross-file ditulis ke $CROSS_FILE"

# 2. Clone Rizin kalau belum ada
if [ ! -d "$SRC_DIR" ]; then
  echo "[2/4] Cloning Rizin..."
  git clone --depth 1 https://github.com/rizinorg/rizin.git "$SRC_DIR"
else
  echo "[2/4] Rizin sudah ada di $SRC_DIR, skip clone."
fi
cd "$SRC_DIR"

# 3. Setup build statis + blob (satu binary, semua tool di-link jadi satu)
echo "[3/4] Konfigurasi meson (static, blob=true)..."
meson setup build \
  --buildtype release \
  --default-library static \
  --prefix "$PREFIX" \
  -Dblob=true \
  -Dstatic_runtime=true \
  --cross-file "$CROSS_FILE"

# 4. Build + install ke PREFIX
echo "[4/4] Building..."
ninja -C build
ninja -C build install

echo ""
echo "Selesai. Binary blob ada di: $PREFIX/bin/"
echo "Salin ke: OmniByte/engine-core/HydraDis/Disassembler/backends/rizin-adapter/prebuilt/arm64-v8a/"