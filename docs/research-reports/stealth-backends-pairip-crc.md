# Stealth Backends: Pairip & CRC Bypass

**Date:** 2026-09-19  
**Module:** ZigZag — `runtime/ZigZag/backends/`  
**Status:** Implemented, NDK compile verified

---

## Overview

Two new stealth backends added to the ZigZag subsystem to complement the existing
Diamorphine (process hiding) and Bypasser (environment detection) backends:

| Backend | Purpose | Technique |
|---------|---------|-----------|
| **Pairip** | Google Play Integrity / Play Protect bypass | GMS property spoofing + DroidGuard interception |
| **CRC** | APK integrity / signature verification bypass | PackageManager hook (JNI/ART interception) |

---

## Pairip — Play Integrity Bypass

### Sources & Citations

| Repository | License | Stars | URL |
|------------|---------|-------|-----|
| KOWX712/PlayIntegrityFix | GPL-3.0 | 3.8k | https://github.com/KOWX712/PlayIntegrityFix |
| dpejoh/specter | GPL-3.0 | — | https://github.com/dpejoh/specter |
| chiteroman/PlayIntegrityFix | GPL-3.0 | archived | https://github.com/chiteroman/PlayIntegrityFix |
| 5ec1cff/TrickyStore | — | — | https://github.com/5ec1cff/TrickyStore |

### Technique

1. **Property Spoofing** — Hook `__system_property_get` in libc via PLT patching.
   Intercepts queries for:
   - `ro.build.fingerprint` → certified Pixel fingerprint
   - `ro.build.version.security_patch` → recent patch level
   - `ro.boot.flash.locked` → `1` (locked bootloader)
   - `ro.build.type` → `user`
   - `ro.debuggable` → `0`
   - `ro.crypto.state` → `encrypted`
   - Device identity → Pixel 6 Pro (`raven`)

2. **DroidGuard Response** — Detect GMS process via `/proc/*/cmdline` scanning.
   When GMS is running, property hook intercepts DroidGuard integrity queries.

3. **Keybox Attestation** — Detect TrickyStore/TEESimulator modules at:
   - `/data/adb/modules/tricky_store`
   - `/data/adb/modules/tee_simulator`

### Integrity Verdicts Targeted

| Verdict | Requirement |
|---------|-------------|
| `MEETS_BASIC_INTEGRITY` | Device not rooted/unlocked |
| `MEETS_DEVICE_INTEGRITY` | Certified device + valid keybox |
| `MEETS_STRONG_INTEGRITY` | Hardware-backed attestation |

### Files

- `backends/Pairip/Pairip.h` — Interface (IStealthBackend)
- `backends/Pairip/Pairip.cpp` — Implementation
- `backends/Pairip/CMakeLists.txt` — Build config v1.0.0

---

## CRC — APK Integrity Bypass

### Sources & Citations

| Repository | License | Stars | URL |
|------------|---------|-------|-----|
| aimardcr/APKKiller | — | 458 | https://github.com/aimardcr/APKKiller |
| L-JINBIN/ApkSignatureKiller | — | 971 | https://github.com/L-JINBIN/ApkSignatureKiller |
| riyadmondol2006/Android-Signature-And-Integrity-Check-Bypass | — | 71 | https://github.com/riyadmondol2006/Android-Signature-And-Integrity-Check-Bypass |
| Android Checksum API | — | — | https://developer.android.com/reference/android/content/pm/Checksum |

### Technique

1. **Signature Verification Hook** — Intercept `PackageManager.getPackageInfo()` via
   ART JNI hooking (`art::JNI::CallObjectMethodV` in `libart.so`). Returns original
   signing certificate when app queries its own signature.

2. **Checksum Verification Hook** — Intercept `PackageManager.requestChecksums()` to
   return valid checksums. Spoofs:
   - `TYPE_WHOLE_MERKLE_ROOT_4K_SHA256` (0x1) — fs-verity, recommended
   - `TYPE_WHOLE_MD5` (0x2) — deprecated, broken
   - `TYPE_WHOLE_SHA1` (0x3) — legacy
   - `TYPE_PARTIAL_MERKLE_ROOT_1M_SHA256` (0x4) — APK Sig V2
   - `TYPE_PARTIAL_MERKLE_ROOT_1M_SHA512` (0x5) — APK Sig V2

3. **Certificate Spoofing** — Read target package signing certificate from
   PackageManager via reflection (`PackageInfo.signingInfo.apkContentsSigners[0].toByteArray()`)

### Files

- `backends/CRC/Crc.h` — Interface (IStealthBackend)
- `backends/CRC/Crc.cpp` — Implementation
- `backends/CRC/CMakeLists.txt` — Build config v1.0.0

---

## Build System Changes

### ZigZag CMakeLists.txt

```cmake
add_subdirectory(backends/Procees/Diamorphine)   # existing
add_subdirectory(backends/LRFP/Bypasser)          # existing
add_subdirectory(backends/Pairip)                  # NEW
add_subdirectory(backends/CRC)                     # NEW

target_link_libraries(zigzag PRIVATE
    common diamorphine_adapter bypasser_adapter
    pairip_adapter crc_adapter                     # NEW
)
```

### Compile Verification

- **NDK:** `/opt/android-ndk-r29` (Clang 21.0.0)
- **Target:** arm64-v8a, API 23, c++_shared
- **Result:** All 7 compilation units pass, `libzigzag.a` linked successfully

---

## Bug Fixes During Implementation

| File | Issue | Fix |
|------|-------|-----|
| `Pairip.cpp` | Missing `<dirent.h>` and `<sys/stat.h>` | Added includes |
| `LRFP.cpp` | Missing `<sys/stat.h>` for `struct stat` | Added include |
| `HideProcess.cpp` | `const char**` vs `const char* const[]` type mismatch | Changed to `const char* const*` |
| `Pairip/CMakeLists.txt` | Wrong include path `../../..` (3 levels, should be 2) | Fixed to `../..` |
| `CRC/CMakeLists.txt` | Same wrong include path | Fixed to `../..` |
| Multiple `.h` files | `#include "../IStealthBackend.h"` — path broke with nesting depth | Changed to `IStealthBackend.h` (resolved via `-I` flag) |

---

## Architecture Notes

All four ZigZag backends implement `IStealthBackend`:

```
IStealthBackend (interface)
├── name(), isAvailable()
├── hide(pid), unhide(pid)
├── bypassPtraceScope(), bypassSelinuxDenial()
└── [backend-specific methods]
```

| Backend | hide()/unhide() | Specific |
|---------|-----------------|----------|
| Diamorphine | ✅ Process hiding via LKM | Module hiding, /proc scan |
| Bypasser | ❌ | Environment detection bypass |
| Pairip | ❌ | Play Integrity, property spoofing |
| CRC | ❌ | APK signature/checksum bypass |
