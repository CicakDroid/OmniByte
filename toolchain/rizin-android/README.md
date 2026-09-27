# rizin-android

Cross-compilation setup for static **rizin** (`librz`) targeting Android ABIs
(`arm64-v8a`, `armeabi-v7a`), consumed by Hydra2D backends via the
`FindRizinAndroid.cmake` module.

## Why rizin v0.9.1

`toolchain/stub-headers/` targets rizin **0.7**, but this build pins **v0.9.1**:

- 0.9.1 is the current stable release line; the stub headers only cover the
  API surface Hydra2D already calls, not a version guarantee.
- The previous `android-arm64.ini` cross-file was also written against a
  newer rizin than 0.7 (meson options like `-Dcli=` did not exist in 0.7).
- Building one pinned tag keeps `prebuilt/` reproducible; bump the tag via
  `RIZIN_TAG=` and rebuild.

The stub headers are **not** modified by this setup.

## Prerequisites

| Tool | Checked via | Notes |
|------|-------------|-------|
| Android NDK r29 | `$NDK` (default `/opt/android-ndk-r29`) | must contain `toolchains/llvm/prebuilt/<host>/bin/clang` |
| meson ≥ 1.0 | `meson` | cross-file syntax used here needs current meson |
| ninja | `ninja` | |
| git | `git` | for cloning rizin |
| cmake ≥ 3.22 | `cmake` | only for the link test / consumer projects |
| network | — | first `meson setup` downloads bundled subproject wraps (capstone, pcre2, lz4, …) |

Host pkg-config is neutralized during the build (`PKG_CONFIG_LIBDIR=/nonexistent`)
so no host libraries leak into the cross build.

## Build

```bash
./build-scripts/build.sh                       # both ABIs
ABIS="arm64-v8a" ./build-scripts/build.sh      # one ABI
JOBS=8 ./build-scripts/build.sh                # raise compile parallelism (default 4)
NDK=/path/to/ndk API=26 RIZIN_TAG=v0.9.1 ./build-scripts/build.sh
```

Output layout:

```
prebuilt/<abi>/
├── include/librz/     # rz_asm.h, rz_analysis.h, ... (+ librz/sdb/ needed by rz_cons.h)
├── lib/               # librz_*.a (static), pkgconfig/
└── share/rizin/       # data files
```

Scratch (clone, meson build dirs, generated cross files) lives in
`.build/` — both `.build/` and `prebuilt/` are gitignored.

### Meson flags used

| Flag | Value | Why |
|------|-------|-----|
| `--default-library static` | — | ship `.a` only; matches how the NDK consumer links |
| `-Dcli=disabled` | feature | no `rizin` CLI binary needed for Android |
| `-Ddebugger=false` | bool | debugger subcommand needs ptrace host tooling |
| `-Dstatic_runtime=true` | bool | static link, no libgcc/libstdc++ shared runtime on device (errors if libs are not static — which is our case) |
| `-Denable_tests=false` | bool | no test executables in a cross build |
| `-Denable_rz_test=false` | bool | rizin's own test runner not built |
| `-Denable_benchmarks=false` | bool | as above |
| `-Denable_examples=false` | bool | as above |

`-Dstatic_runtime=true` is only valid with `--default-library static`;
meson errors out otherwise (checked in `meson.build`).

## Link test

```bash
cmake -S test -B .build/link-test-arm64 \
  -DCMAKE_TOOLCHAIN_FILE=$NDK/build/cmake/android.toolchain.cmake \
  -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-24
cmake --build .build/link-test-arm64
```

`test/test_rz_asm.c` calls `rz_asm_new()` / `rz_asm_free()` and must link
against `RizinAndroid::Rizin`. Verify the artifact with
`llvm-readelf -h` (Machine: AArch64 / ARM) — it cannot be executed on the
build host.

## Consuming from CMake

```cmake
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/toolchain/rizin-android")
set(RIZIN_ANDROID_ROOT "${CMAKE_SOURCE_DIR}/toolchain/rizin-android")  # default
set(ANDROID_ABI "arm64-v8a")                                           # default
find_package(RizinAndroid REQUIRED)
target_link_libraries(my_target PRIVATE RizinAndroid::Rizin)
```

`RizinAndroid::Rizin` is an INTERFACE target carrying:

- include dirs: `include/librz` **and** `include/librz/sdb`
  (`rz_cons.h` does `#include <sdb.h>`, same as `rz_util.pc` Cflags)
- link line: all `lib/*.a` wrapped in `-Wl,--start-group ... -Wl,--end-group`
  (librz archives reference each other cyclically) plus `m` and `dl`

Static builds do **not** generate `RizinConfig.cmake` (rizin only installs it
for shared builds), which is exactly why this find module exists.

## Layout

```
rizin-android/
├── cross-files/            # meson cross-file templates (TOOLCHAIN/API placeholders)
│   ├── android-arm64.txt
│   └── android-armv7.txt
├── build-scripts/build.sh  # clone + meson setup + compile + install per ABI
├── FindRizinAndroid.cmake  # RizinAndroid::Rizin imported target
├── test/                   # link test (rz_asm_new)
├── prebuilt/               # build output (gitignored)
└── .build/                 # scratch: source clone, meson dirs (gitignored)
```
