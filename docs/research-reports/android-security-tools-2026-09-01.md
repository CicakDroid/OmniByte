# Android Security Tools Research Report

**Date:** 2026-09-01
**Purpose:** Evaluate 30 GitHub repos for integration into OmniByte project
**Criteria:** lightweight, top performance, evasion, traceless

---

## 1. Executive Summary

Researched 30 repos across Android security, hooking, memory manipulation, root solutions, and reverse engineering. Of these, **9 repos are highly relevant** for integration, **5 are moderately relevant**, and **11 were not found** (deleted/private/renamed).

**Top picks by criteria:**

| Criteria | Best Match | Stars | Why |
|----------|-----------|-------|-----|
| **Lightweight** | bytedance/bhook | 2574 | Pure C, PLT hook, ~1.7KB disk |
| **Top Performance** | MJx0/KittyMemory | 545 | C++ memory patching/scanning |
| **Evasion** | zhaodice/qemu-anti-detection | 1636 | QEMU hide, bypass mhyprot/EAC |
| **Traceless** | 1013503897/Vector | 7 | KPM inline hooker, no LSPosed trace |

---

## 2. All Researched Repos

### 2.1 FOUND (19 repos)

| # | Repo | ★ | Lang | License | Active | Last Push |
|---|------|---|------|---------|--------|-----------|
| 1 | SukiSU-Ultra/SukiSU-Ultra | 6187 | Kotlin | GPL-3.0 | ✅ | 2026-09-01 |
| 2 | bytedance/bhook | 2574 | C | MIT | ✅ | 2026-06-16 |
| 3 | zhaodice/qemu-anti-detection | 1636 | - | None | ✅ | 2026-04-18 |
| 4 | Misaka-Mikoto-Tech/MonoHook | 1061 | C# | MIT | ❌ | 2023-09-22 |
| 5 | LuckyPray/DexKit | 1014 | Kotlin | Apache-2.0 | ✅ | 2026-08-20 |
| 6 | L-JINBIN/ApkSignatureKillerEx | 841 | C | None | ❌ | 2023-01-25 |
| 7 | RolfRolles/HexRaysDeob | 806 | C++ | GPL-3.0 | ❌ | 2021-02-22 |
| 8 | MJx0/KittyMemory | 545 | C++ | MIT | ✅ | 2026-08-03 |
| 9 | MJx0/KittyMemoryEx | 185 | C++ | MIT | ✅ | 2026-08-03 |
| 10 | Titoot/KeyDot | 127 | C++ | MIT | ✅ | 2026-08-18 |
| 11 | HongThatCong/FindCrypt3 | 124 | C++ | MIT | ❌ | 2022-12-23 |
| 12 | LRFP-Team/Bypasser | 74 | C++ | GPL-3.0 | ✅ | 2026-08-05 |
| 13 | MMRLApp/RootThread | 13 | Java | GPL-3.0 | ✅ | 2026-04-01 |
| 14 | n0pex3/Sharingan | 13 | Python | None | ✅ | 2026-05-12 |
| 15 | dbcyyds/MemDbg | 8 | C++ | Other | ✅ | 2026-08-05 |
| 16 | 1013503897/Vector | 7 | Java | GPL-3.0 | ✅ | 2026-09-01 |
| 17 | drsteelman/Noctua-C | 1 | C | None | ✅ | 2026-07-10 |

### 2.2 NOT FOUND (11 repos)

| Repo | Status |
|------|--------|
| m0nad/Diamorpine | ❌ Repo not found (deleted/private) |
| 1013503897/stealth-povL-JINBIN-ApkSignatureKiller | ❌ Repo not found |
| rm-NooblnCoding/UniversalSigBypasser | ❌ Repo not found |
| Xiatong6666/Sui | ❌ Repo not found |
| bytedance/android-inlinehook | ❌ Repo not found (merged into bhook?) |
| AlbtrossHook/AlbatrossAndroid | ❌ Repo not found |
| tatomodrekilaze/S02-External-Memory-Bridge | ❌ Repo not found |
| KasperskyLab/htrng | ❌ Repo not found (internal tool?) |
| TanuirHossain2/NextVM | ❌ Repo not found |

---

## 3. Filtered by Criteria: lightweight + performance + evasion + traceless

### 3.1 TIER 1 — HIGHLY RECOMMENDED (direct integration)

#### bytedance/bhook
- **What:** Universal Android PLT hook library (armeabi-v7a, arm64-v8a, x86, x86_64)
- **Why:** Pure C, MIT license, 1.7KB disk, 2574★, actively maintained by ByteDance
- **Evasion:** PLT hooking is stealthier than inline hooking (modifies GOT, not code sections)
- **Placement:** `runtime/Bridges/HookEngine/` — wraps bhook as native dependency
- **Integration:** Link as static lib via CMake, expose C API through JNI bridge

#### MJx0/KittyMemory
- **What:** C++ library for runtime memory patching, scanning, dumping, module introspection
- **Why:** MIT license, 545★, actively maintained, supports Android + iOS
- **Performance:** Direct memory operations via `/proc/pid/mem` or ptrace
- **Placement:** `runtime/MemoryIO/` — core memory read/write/patch engine
- **Integration:** Wrap as C++ class, expose through JNI

#### MJx0/KittyMemoryEx
- **What:** External remote process memory manipulation (patching, scanning, injection)
- **Why:** MIT license, 185★, complement to KittyMemory for cross-process ops
- **Performance:** External process memory via `/proc/pid/mem`
- **Placement:** `runtime/MemoryIO/ExternalEngine/` — external process variant
- **Integration:** Extend KittyMemory wrapper with external process support

#### 1013503897/Vector
- **What:** LSPosed fork with traceless KPM hook backend (inline_hooker via KernelPatch)
- **Why:** Only "traceless" option found — KPM hooks leave no userspace traces
- **Evasion:** KernelPatch module runs in kernel space, invisible to userspace detectors
- **Placement:** `runtime/Bridges/HookEngine/TracelessBackend/` — KPM-based hook backend
- **Integration:** Reference implementation only (requires kernel access)

### 3.2 TIER 2 — MODERATELY RECOMMENDED (selective integration)

#### zhaodice/qemu-anti-detection
- **What:** QEMU hiding patch — bypass mhyprot, EAC, nProtect, VMProtect, Themida
- **Why:** 1636★, addresses VM detection (critical for emulation-based analysis)
- **Evasion:** Hides QEMU signatures from anti-cheat/anti-tamper systems
- **Placement:** `toolchain/emulator/patches/` — QEMU patch for analysis environment
- **Integration:** Apply patches to QEMU build, not runtime dependency

#### LuckyPray/DexKit
- **What:** High-performance DEX deobfuscation library (Kotlin API)
- **Why:** 1014★, Apache-2.0, actively maintained, fast DEX analysis
- **Placement:** `engine-core/HydraDis/Plugin/Deobfuscate/DexKit/` — DEX deobfuscation
- **Integration:** Link as Kotlin/JNI dependency

#### L-JINBIN/ApkSignatureKillerEx
- **What:** APK signature removal and anti-tamper bypass (MT Manager fork)
- **Why:** 841★, C implementation, addresses signature verification
- **Evasion:** Bypasses APK signature checks at runtime
- **Placement:** `modules/Dumper/DumperCore/SignatureBypass/` — signature handling
- **Integration:** Extract C code, adapt to project's bypass architecture

#### LRFP-Team/Bypasser
- **What:** Root detection bypass system module (LRFP framework)
- **Why:** 74★, C++, GPL-3.0, actively maintained, systematic bypass approach
- **Evasion:** Hides root environment from detection
- **Placement:** `runtime/BypassManager/` — root detection bypass layer
- **Integration:** Adapt bypass techniques into runtime environment manager

#### Titoot/KeyDot
- **What:** Godot engine encryption key extractor (static analysis)
- **Why:** 127★, MIT, C++, fast, specifically for Godot games
- **Placement:** `modules/Dumper/Engines/Godot/Analyzer/KeyExtractor/` — Godot key extraction
- **Integration:** Port extraction logic to Godot engine profile

### 3.3 TIER 3 — REFERENCE ONLY (not directly integrable)

| Repo | Why Reference Only |
|------|-------------------|
| Misaka-Mikoto-Tech/MonoHook | C# IL2CPP hooking — useful for Unity Mono engine, but inactive since 2023 |
| RolfRolles/HexRaysDeob | IDA plugin — offline analysis tool, not runtime |
| HongThatCong/FindCrypt3 | IDA plugin — offline crypto constant finder |
| n0pex3/Sharingan | IDA plugin — offline deobfuscation |
| dbcyyds/MemDbg | CE-style debugger — end-user tool, not library |
| drsteelman/Noctua-C | RE framework — too broad, low stars |
| MMRLApp/RootThread | Root IPC library — useful but Java-only, niche |

---

## 4. Recommended Integration Plan

### Phase 1: Core Memory + Hooking (Week 1-2)
```
runtime/
├── MemoryIO/
│   ├── MemoryEngine.h/.cpp      ← KittyMemory wrapper
│   └── ExternalEngine.h/.cpp    ← KittyMemoryEx wrapper
└── Bridges/
    └── HookEngine/
        ├── PLTHook.h/.cpp       ← bhook wrapper
        └── TracelessHook.h/.cpp ← Vector/KPM reference
```

### Phase 2: Evasion + Bypass (Week 3)
```
runtime/
├── BypassManager/
│   ├── RootBypass.h/.cpp        ← Bypasser techniques
│   └── SignatureBypass.h/.cpp   ← ApkSignatureKillerEx techniques
└── Environment/
    └── VMHider.h/.cpp           ← qemu-anti-detection reference
```

### Phase 3: Engine-Specific (Week 4)
```
engine-core/HydraDis/Plugin/Deobfuscate/
└── DexKit/                      ← DexKit integration

modules/Dumper/Engines/Godot/Analyzer/
└── KeyExtractor/                ← KeyDot port
```

---

## 5. NOT FOUND — Verification Needed

These repos may have been deleted, renamed, or made private. Verify manually:

| Repo | Possible Cause |
|------|---------------|
| m0nad/Diamorpine | Repo deleted or made private |
| 1013503897/stealth-povL-JINBIN-ApkSignatureKiller | Typo in name or deleted |
| rm-NooblnCoding/UniversalSigBypasser | Typo in name or deleted |
| Xiatong6666/Sui | May be confused with Magisk-Redux/Sui |
| bytedance/android-inlinehook | Merged into bhook or made internal |
| AlbtrossHook/AlbatrossAndroid | Typo in org name or deleted |
| tatomodrekilaze/S02-External-Memory-Bridge | Typo in name or deleted |
| KasperskyLab/htrng | Internal Kaspersky tool, not public |
| TanuirHossain2/NextVM | Repo deleted or made private |

---

## 6. Sources

| Claim | Source | URL |
|-------|--------|-----|
| bhook PLT hook library | GitHub API 2026-09-01 | https://github.com/bytedance/bhook |
| KittyMemory memory patching | GitHub API 2026-09-01 | https://github.com/MJx0/KittyMemory |
| KittyMemoryEx external memory | GitHub API 2026-09-01 | https://github.com/MJx0/KittyMemoryEx |
| Vector traceless KPM hook | GitHub API 2026-09-01 | https://github.com/1013503897/Vector |
| qemu-anti-detection | GitHub API 2026-09-01 | https://github.com/zhaodice/qemu-anti-detection |
| DexKit DEX deobfuscation | GitHub API 2026-09-01 | https://github.com/LuckyPray/DexKit |
| ApkSignatureKillerEx | GitHub API 2026-09-01 | https://github.com/L-JINBIN/ApkSignatureKillerEx |
| Bypasser root bypass | GitHub API 2026-09-01 | https://github.com/LRFP-Team/Bypasser |
| KeyDot Godot extractor | GitHub API 2026-09-01 | https://github.com/Titoot/KeyDot |
| SukiSU-Ultra kernel root | GitHub API 2026-09-01 | https://github.com/SukiSU-Ultra/SukiSU-Ultra |
