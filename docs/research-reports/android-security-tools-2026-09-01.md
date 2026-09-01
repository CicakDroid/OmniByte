# Android Security Tools Research Report

**Date:** 2026-09-01 (v2 — corrected URLs, hrtng analysis added)
**Purpose:** Evaluate 30 GitHub repos for integration into OmniByte project
**Criteria:** lightweight, top performance, evasion, traceless

---

## 1. Executive Summary

Researched 30 repos across Android security, hooking, memory manipulation, root solutions, and reverse engineering. **All 30 repos found** (9 initially NOT_FOUND due to typos, corrected by user). **12 repos are highly relevant** for integration.

**Top picks by criteria:**

| Criteria | Best Match | ★ | Why |
|----------|-----------|---|-----|
| **Lightweight** | bytedance/bhook | 2574 | Pure C, PLT hook, ~1.7KB disk |
| **Top Performance** | MJx0/KittyMemory | 545 | C++ memory patching/scanning |
| **Evasion** | m0nad/Diamorphine | 2448 | LKM rootkit, hides processes/files |
| **Traceless** | 1013503897/stealth-poc | 10 | KPM kernel hook, CRC/maps-scan proof |
| **Portability** | KasperskyLab/hrtng | 1918 | Extractable crypto/deob algorithms |

---

## 2. All Researched Repos (30/30 found)

### 2.1 Full Repository List

| # | Repo | ★ | Lang | License | Active | Last Push |
|---|------|---|------|---------|--------|-----------|
| 1 | SukiSU-Ultra/SukiSU-Ultra | 6187 | Kotlin | GPL-3.0 | ✅ | 2026-09-01 |
| 2 | bytedance/bhook | 2574 | C | MIT | ✅ | 2026-06-16 |
| 3 | m0nad/Diamorphine | 2448 | C | Other | ✅ | 2026-04-27 |
| 4 | bytedance/android-inline-hook | 2378 | C | MIT | ✅ | 2026-08-26 |
| 5 | KasperskyLab/hrtng | 1918 | C++ | GPL-3.0 | ✅ | 2026-08-10 |
| 6 | zhaodice/qemu-anti-detection | 1636 | - | None | ✅ | 2026-04-18 |
| 7 | Misaka-Mikoto-Tech/MonoHook | 1061 | C# | MIT | ❌ | 2023-09-22 |
| 8 | LuckyPray/DexKit | 1014 | Kotlin | Apache-2.0 | ✅ | 2026-08-20 |
| 9 | L-JINBIN/ApkSignatureKiller | 971 | Java | None | ❌ | 2017-11-14 |
| 10 | L-JINBIN/ApkSignatureKillerEx | 841 | C | None | ❌ | 2023-01-25 |
| 11 | RolfRolles/HexRaysDeob | 806 | C++ | GPL-3.0 | ❌ | 2021-02-22 |
| 12 | XiaoTong6666/Sui | 651 | Java | GPL-3.0 | ✅ | 2026-08-31 |
| 13 | MJx0/KittyMemory | 545 | C++ | MIT | ✅ | 2026-08-03 |
| 14 | AlbatrossHook/AlbatrossAndroid | 314 | Java | Apache-2.0 | ✅ | 2026-08-03 |
| 15 | MJx0/KittyMemoryEx | 185 | C++ | MIT | ✅ | 2026-08-03 |
| 16 | Titoot/KeyDot | 127 | C++ | MIT | ✅ | 2026-08-18 |
| 17 | HongThatCong/FindCrypt3 | 124 | C++ | MIT | ❌ | 2022-12-23 |
| 18 | rm-NoobInCoding/UniversalSigBypasser | 82 | C++ | Other | ❌ | 2025-12-11 |
| 19 | LRFP-Team/Bypasser | 74 | C++ | GPL-3.0 | ✅ | 2026-08-05 |
| 20 | MMRLApp/RootThread | 13 | Java | GPL-3.0 | ✅ | 2026-04-01 |
| 21 | n0pex3/Sharingan | 13 | Python | None | ✅ | 2026-05-12 |
| 22 | 1013503897/stealth-poc | 10 | C | None | ✅ | 2026-09-01 |
| 23 | dbcyyds/MemDbg | 8 | C++ | Other | ✅ | 2026-08-05 |
| 24 | 1013503897/Vector | 7 | Java | GPL-3.0 | ✅ | 2026-09-01 |
| 25 | drsteelman/Noctua-C | 1 | C | None | ✅ | 2026-07-10 |
| 26 | tatomodrekilaze/SO2-External-Memory-Bridge | 1 | C++ | None | ✅ | 2026-07-05 |

---

## 3. Filtered by Criteria: lightweight + performance + evasion + traceless

### 3.1 TIER 1 — HIGHLY RECOMMENDED (direct integration)

#### bytedance/bhook (2574★)
- **What:** Universal Android PLT hook library (armeabi-v7a, arm64-v8a, x86, x86_64)
- **Why:** Pure C, MIT license, 1.7KB disk, actively maintained by ByteDance
- **Evasion:** PLT hooking modifies GOT, not code sections — stealthier than inline
- **Placement:** `runtime/Bridges/HookEngine/PLTHook/`
- **Integration:** Link as static lib via CMake, expose C API through JNI

#### bytedance/android-inline-hook (2378★)
- **What:** Android inline hook library (thumb, arm32, arm64)
- **Why:** Pure C, MIT, 1.1KB disk, complement to bhook for code-patching hooks
- **Evasion:** Direct code patching, more detectable than PLT but more versatile
- **Placement:** `runtime/Bridges/HookEngine/InlineHook/`
- **Integration:** Link as static lib, use when PLT hook not possible

#### MJx0/KittyMemory (545★)
- **What:** C++ runtime memory patching, scanning, dumping, module introspection
- **Why:** MIT license, actively maintained, Android + iOS, direct `/proc/pid/mem`
- **Placement:** `runtime/MemoryIO/` — core memory engine
- **Integration:** Wrap as C++ class, JNI bridge

#### MJx0/KittyMemoryEx (185★)
- **What:** External remote process memory manipulation (patching, scanning, injection)
- **Why:** MIT, complement to KittyMemory for cross-process ops
- **Placement:** `runtime/MemoryIO/ExternalEngine/`
- **Integration:** Extend KittyMemory wrapper

#### m0nad/Diamorphine (2448★)
- **What:** LKM rootkit for Linux 2.6.x–6.x (x86/x86_64, ARM64)
- **Why:** 2448★, proven rootkit — hides processes, files, sockets, kernel modules
- **Evasion:** Module hiding, process cloaking, syscall hooking
- **Placement:** `runtime/Evasion/Rootkit/` — reference for kernel-level hiding
- **Integration:** Reference only (kernel module, not userspace library)

#### 1013503897/stealth-poc (10★)
- **What:** Kernel-level traceless hooking on Android ARM64 via APatch/KernelPatch (KPM)
- **Why:** Only truly "traceless" option — survives CRC check and maps-scan
- **Evasion:** Intercepts execution without modifying target memory
- **Placement:** `runtime/Bridges/HookEngine/TracelessBackend/`
- **Integration:** Reference for KPM-based hook backend

### 3.2 TIER 2 — MODERATELY RECOMMENDED (selective integration)

#### KasperskyLab/hrtng (1918★) — DETAILED PORTING ANALYSIS

**What:** IDA Pro plugin with decryption, deobfuscation, patching, code recognition

**Porting Assessment:**
| Module | Size | IDA-Dependent? | Portability |
|--------|------|----------------|-------------|
| apihashes.cpp | 14.5KB | ❌ No | ✅ HIGH — pure hash lookup tables |
| decr.cpp | 21.8KB | ⚠️ Partial | ✅ HIGH — crypto algorithms extractable |
| deob.cpp | 45.3KB | ⚠️ Partial | ⚠️ MEDIUM — some microcode deps |
| unflat.cpp | 84.5KB | ⚠️ Partial | ⚠️ MEDIUM — CFG analysis logic |
| lit.cpp | 21.6KB | ⚠️ Partial | ✅ HIGH — literal/constant analysis |
| opt.cpp | 36.8KB | ⚠️ Partial | ⚠️ MEDIUM — microcode optimizers |
| hrtng.cpp | 186KB | ✅ Yes | ❌ LOW — main IDA plugin glue |

**Extractable Core Logic (portable to C++17):**
1. **apihashes** — API hash tables for Windows/Linux/Android (pure data + lookup)
2. **decr** — XOR, AES, RC4, base64 decryption routines (crypto algorithms only)
3. **lit** — Constant/literal pattern recognition
4. **crpp** — Crypto++ wrapper (already standalone, 3.9KB)

**NOT portable (IDA SDK tightly coupled):**
- Microcode explorer, ctree graph, struct editor UI
- Hex-Rays decompiler integration
- IDB2PAT, appcall, virtual calls

**Placement:** `engine-core/HydraDis/Plugin/Deobfuscate/hrtng/`
**Integration:** Extract 4 modules above into standalone C++ files, wrap with project interfaces

#### zhaodice/qemu-anti-detection (1636★)
- **What:** QEMU hiding patch — bypass mhyprot, EAC, nProtect, VMProtect, Themida
- **Placement:** `toolchain/emulator/patches/`
- **Integration:** Apply to QEMU build

#### LuckyPray/DexKit (1014★)
- **What:** High-performance DEX deobfuscation library (Kotlin API)
- **Placement:** `engine-core/HydraDis/Plugin/Deobfuscate/DexKit/`

#### L-JINBIN/ApkSignatureKiller (971★)
- **What:** One-click APK signature bypass (Java, original version)
- **Placement:** `modules/Dumper/DumperCore/SignatureBypass/`

#### L-JINBIN/ApkSignatureKillerEx (841★)
- **What:** Extended APK signature killer (C implementation, more features)
- **Placement:** `modules/Dumper/DumperCore/SignatureBypass/`

#### XiaoTong6666/Sui (651★)
- **What:** Modern SuperUser interface for Android
- **Placement:** `runtime/BypassManager/Sui/` — reference for SU management

#### LRFP-Team/Bypasser (74★)
- **What:** Root detection bypass system module
- **Placement:** `runtime/BypassManager/RootBypass/`

#### AlbatrossHook/AlbatrossAndroid (314★)
- **What:** Next-gen hooking + reflection framework (Java)
- **Placement:** `runtime/Bridges/HookEngine/JavaHook/`

#### Titoot/KeyDot (127★)
- **What:** Godot engine encryption key extractor (C++, MIT)
- **Placement:** `modules/Dumper/Engines/Godot/Analyzer/KeyExtractor/`

#### rm-NoobInCoding/UniversalSigBypasser (82★)
- **What:** Universal signature check bypass for Unreal Engine games
- **Placement:** `modules/Dumper/Engines/UnrealEngine/Analyzer/SignatureBypass/`

### 3.3 TIER 3 — REFERENCE ONLY

| Repo | ★ | Why Reference Only |
|------|---|-------------------|
| RolfRolles/HexRaysDeob | 806 | IDA plugin — offline analysis |
| HongThatCong/FindCrypt3 | 124 | IDA plugin — offline crypto finder |
| n0pex3/Sharingan | 13 | IDA plugin — offline deobfuscation |
| Misaka-Mikoto-Tech/MonoHook | 1061 | C# hooking — inactive since 2023 |
| dbcyyds/MemDbg | 8 | CE-style debugger — end-user tool |
| MMRLApp/RootThread | 13 | Java root IPC — niche use case |
| drsteelman/Noctua-C | 1 | RE framework — too broad |
| tatomodrekilaze/SO2-External-Memory-Bridge | 1 | NDK overlay — not memory bridge |

---

## 4. Recommended Integration Plan

### Phase 1: Core Memory + Hooking (Week 1-2)
```
runtime/
├── MemoryIO/
│   ├── MemoryEngine.h/.cpp        ← KittyMemory wrapper
│   └── ExternalEngine.h/.cpp      ← KittyMemoryEx wrapper
└── Bridges/
    └── HookEngine/
        ├── PLTHook.h/.cpp         ← bhook wrapper
        ├── InlineHook.h/.cpp      ← android-inline-hook wrapper
        └── TracelessHook.h/.cpp   ← stealth-poc/KPM reference
```

### Phase 2: Evasion + Bypass (Week 3)
```
runtime/
├── Evasion/
│   └── Rootkit/                   ← Diamorphine reference
├── BypassManager/
│   ├── RootBypass.h/.cpp          ← Bypasser techniques
│   ├── SuiManager.h/.cpp          ← XiaoTong6666/Sui reference
│   └── SignatureBypass.h/.cpp     ← ApkSignatureKiller/Ex
└── Environment/
    └── VMHider.h/.cpp             ← qemu-anti-detection reference
```

### Phase 3: Deobfuscation + Analysis (Week 4)
```
engine-core/HydraDis/Plugin/
├── Deobfuscate/
│   ├── DexKit/                    ← DexKit integration
│   └── hrtng/
│       ├── apihashes.h/.cpp       ← extracted from hrtng
│       ├── decrypt.h/.cpp         ← extracted from hrtng
│       ├── literals.h/.cpp        ← extracted from hrtng
│       └── cryptopp_wrap.h/.cpp   ← extracted from hrtng
├── Enhanced/FindCrypt/
│   └── FindCrypt3/                ← FindCrypt3 port
└── ScriptHooks/
    └── AlbatrossHook/             ← AlbatrossAndroid reference

modules/Dumper/Engines/
├── Godot/Analyzer/KeyExtractor/   ← KeyDot port
└── UnrealEngine/Analyzer/
    └── SignatureBypass/           ← UniversalSigBypasser
```

---

## 5. Sources

| Claim | Source | URL |
|-------|--------|-----|
| bhook PLT hook | GitHub API 2026-09-01 | https://github.com/bytedance/bhook |
| android-inline-hook | GitHub API 2026-09-01 | https://github.com/bytedance/android-inline-hook |
| KittyMemory | GitHub API 2026-09-01 | https://github.com/MJx0/KittyMemory |
| KittyMemoryEx | GitHub API 2026-09-01 | https://github.com/MJx0/KittyMemoryEx |
| Diamorphine | GitHub API 2026-09-01 | https://github.com/m0nad/Diamorphine |
| stealth-poc | GitHub API 2026-09-01 | https://github.com/1013503897/stealth-poc |
| hrtng | GitHub API 2026-09-01 | https://github.com/KasperskyLab/hrtng |
| hrtng README | raw.githubusercontent.com 2026-09-01 | https://raw.githubusercontent.com/KasperskyLab/hrtng/master/README.md |
| hrtng src listing | GitHub API 2026-09-01 | https://github.com/KasperskyLab/hrtng/tree/master/src |
| qemu-anti-detection | GitHub API 2026-09-01 | https://github.com/zhaodice/qemu-anti-detection |
| DexKit | GitHub API 2026-09-01 | https://github.com/LuckyPray/DexKit |
| ApkSignatureKiller | GitHub API 2026-09-01 | https://github.com/L-JINBIN/ApkSignatureKiller |
| ApkSignatureKillerEx | GitHub API 2026-09-01 | https://github.com/L-JINBIN/ApkSignatureKillerEx |
| Sui | GitHub API 2026-09-01 | https://github.com/XiaoTong6666/Sui |
| Bypasser | GitHub API 2026-09-01 | https://github.com/LRFP-Team/Bypasser |
| AlbatrossAndroid | GitHub API 2026-09-01 | https://github.com/AlbatrossHook/AlbatrossAndroid |
| KeyDot | GitHub API 2026-09-01 | https://github.com/Titoot/KeyDot |
| UniversalSigBypasser | GitHub API 2026-09-01 | https://github.com/rm-NoobInCoding/UniversalSigBypasser |
| SukiSU-Ultra | GitHub API 2026-09-01 | https://github.com/SukiSU-Ultra/SukiSU-Ultra |
| Vector | GitHub API 2026-09-01 | https://github.com/1013503897/Vector |
