# Laporan Penelitian: Reverse Engineering — Workflow, Tools, Root Access & Implementasi Mod Menu

**Nama Proyek:** Pengembangan OmniByte
**Tanggal:** 2026-09-05
**Status:** Final
**Revisi:** 4.0 — Reorganisasi Workflow RE + Penambahan Tools, Root Access, Protobuf, SQL Hook, Mod Menu

---

## Daftar Isi

### Part I — Workflow Reverse Engineering (Phase-Based)

1. [Phase 1: Reconnaissance & Extraction](#phase-1-reconnaissance--extraction)
2. [Phase 2: Static Analysis](#phase-2-static-analysis)
3. [Phase 3: Dynamic Analysis](#phase-3-dynamic-analysis)
4. [Phase 4: Vulnerability Research & Fuzzing](#phase-4-vulnerability-research--fuzzing)
5. [Phase 5: Root Access & Kernel Patching](#phase-5-root-access--kernel-patching)
6. [Phase 6: Stealth, Evasion & Anti-Analysis](#phase-6-stealth-evasion--anti-analysis)
7. [Phase 7: Implementation — Mod Menu & Hook Code Generation](#phase-7-implementation--mod-menu--hook-code-generation)
8. [Phase 8: Output, Packaging & Distribution](#phase-8-output-packaging--distribution)

### Part II — Referensi Tools & Istilah Lengkap

9. [APK Extraction Tools](#9-apk-extraction-tools)
10. [Static Analysis Tools](#10-static-analysis-tools)
11. [Dynamic Analysis & Hooking Frameworks](#11-dynamic-analysis--hooking-frameworks)
12. [Fuzzing Engines](#12-fuzzing-engines)
13. [Network & Traffic Analysis](#13-network--traffic-analysis)
14. [Memory Forensics](#14-memory-forensics)
15. [Protobuf Reverse Engineering](#15-protobuf-reverse-engineering)
16. [SQL Hook & Database Manipulation](#16-sql-hook--database-manipulation)
17. [Metadata & Symbol Recovery](#17-metadata--symbol-recovery)

### Part III — Konsep Inti Reverse Engineering

18. [Synthesizer](#18-synthesizer)
19. [Taint Analysis](#19-taint-analysis)
20. [Expression Synthesis](#20-expression-synthesis)
21. [Memory Segmentation](#21-memory-segmentation)
22. [Coverage Strategies](#22-coverage-strategies)
23. [IR (Intermediate Representation)](#23-ir-intermediate-representation)
24. [Entropy](#24-entropy)
25. [Packer & Recursive Binary Unpacker](#25-packer--recursive-binary-unpacker)
26. [Section Carver](#26-section-carver)
27. [Compiler Analysis](#27-compiler-analysis)
28. [Stacktrace](#28-stacktrace)

### Part IV — Pertahanan & Anti-Analysis

29. [Anti Debug](#29-anti-debug)
30. [Anti Cheat](#30-anti-cheat)
31. [Anti-Tampering](#31-anti-tampering)
32. [Root Checker](#32-root-checker)
33. [SSL Unpinning](#33-ssl-unpinning)
34. [Play Integrity Check](#34-play-integrity-check)
35. [Deobfuscation](#35-deobfuscation)
36. [Hooking Mechanisms](#36-hooking-mechanisms)
37. [Signature & Magic Number](#37-signature--magic-number)

### Part V — Konsep Lanjutan

38. [Sanitizer Mechanism](#38-sanitizer-mechanism)
39. [Basic Heap Allocator](#39-basic-heap-allocator)
40. [Pointer Coverage](#40-pointer-coverage)
41. [Metadata IL2CPP/globalmetadata.dat](#41-metadata-il2cppglobalmetadatadat)

### Part VI — Root Access Ecosystem

42. [SukiSU-Ultra](#42-sukisu-ultra)
43. [Libsu](#43-libsu)
44. [Kernel Driver untuk Android Rooting](#44-kernel-driver-untuk-android-rooting)
45. [xdl (Extended Dynamic Linker)](#45-xdl-extended-dynamic-linker)
46. [Varian Root Lainnya](#46-varian-root-lainnya)

### Part VII — Android Mod Menu

47. [Mod Menu Architecture](#47-mod-menu-architecture)
48. [Hook Code Generation](#48-hook-code-generation)
49. [Target Fungsi dalam Game](#49-target-fungsi-dalam-game)
50. [Mod Menu Implementation Flow](#50-mod-menu-implementation-flow)

51. [Kesimpulan & Relevansi untuk OmniByte](#51-kesimpulan--relevansi-untuk-omnibyte)
52. [Daftar Pustaka & Sitasi](#52-daftar-pustaka--situsi)

---

# Part I — Workflow Reverse Engineering (Phase-Based)

## Phase 1: Reconnaissance & Extraction

### Tujuan
Mengidentifikasi format binary, mengekstrak konten APK, dan memahami struktur awal target.

### Alat Utama

| Tool | Fungsi | Output |
|------|--------|--------|
| **Apktool** | Decompile APK → smali + resources | Directory struktur APK |
| **dex2jar** | DEX bytecode → JAR class files | `.jar` untuk decompiler |
| **JADX** | DEX/APK → Java source code | Source code Java |
| **AXMLPrinter** | Binary AndroidManifest.xml → teks | XML readable |
| **apk-parser** | Parse APK tanpa decompile penuh | Metadata ringkas |
| **binwalk** | Scan magic bytes & embed files | Firmware/components |
| **file** (Linux) | Identifikasi file type via magic | Tipe format |
| **7z/unzip** | Ekstrak archive (APK = ZIP) | File contents |

### Workflow Extraction

```
target.apk
    │
    ├── [Apktool] → smali/, res/, AndroidManifest.xml
    ├── [dex2jar] → target-dex2jar.jar
    ├── [JADX] → src/ (Java source)
    └── [manual unzip] → classes.dex, lib/, assets/, META-INF/
```

### Referensi
- iBotPeaches, "Apktool: A tool for reverse engineering Android APK files," GitHub, https://github.com/iBotPeaches/Apktool
- Google, "Android Binary XML (AXML) Format," https://android.googlesource.com/platform/frameworks/base/+/master/libs/androidfw/README
- CanadaHonk, "JADX - Dex to Java decompiler," https://github.com/skylot/jadx

---

## Phase 2: Static Analysis

### Tujuan
Memahami kode tanpa mengeksekusinya. Menganalisis alur kontrol, aliran data, dan identifikasi fungsi target.

### Alat Utama

| Tool | Tipe | Karakteristik |
|------|------|---------------|
| **Ghidra** | Disassembler + Decompiler | P-code IR, open source (NSA) |
| **IDA Pro** | Disassembler + Decompiler | Industry standard, FLIRT signatures |
| **radare2/rizin** | RE Framework | ESIL VM, scriptable, CLI-based |
| **Binary Ninja** | Disassembler + Decompiler | IL modern, API-first |
| **Capstone** | Disassembly Framework | Multi-arch, embeddable |
| **IL2CppDumper** | Metadata Extractor | Unity IL2CPP → C# stubs |

### Metode Static Analysis

1. **Signature Scanning**: Identifikasi compiler/library dari byte pattern
2. **Import/Export Analysis**: Pahami dependency dan API calls
3. **Control Flow Graph (CFG)**: Visualisasi alur eksekusi
4. **String Analysis**: Temukan URL, API keys, error messages
5. **Cross-Reference (XRef)**: Trace penggunaan fungsi/variabel

### Referensi
- National Security Agency, "Ghidra: A Software Reverse Engineering Framework," https://ghidra-sre.org/
- Hex-Rays, "IDA Pro: Multi-processor disassembler and debugger," https://hex-rays.com/ida-pro/
- radareorg, "radare2: UNIX-like reverse engineering framework," https://github.com/radareorg/radare2

---

## Phase 3: Dynamic Analysis

### Tujuan
Menganalisis perilaku program saat runtime. Menginstall hooks, memantau memory, dan meng-intercept komunikasi.

### Alat Utama

| Tool | Mekanisme | Keunggulan |
|------|-----------|------------|
| **Frida** | JavaScript injection ke process | Cross-platform, real-time, scriptable |
| **Xposed Framework** | Hook method di ART runtime | Persistent, system-wide hooks |
| **Objection** | Runtime mobile exploration | High-level API, cepat |
| **Cydia Substrate** | Code patching platform | Mature, foundation untuk banyak tools |
| **Androguard** | Static + dynamic analysis | Python-based, integrasi mudah |

### Metode Dynamic Analysis

1. **Method Hooking**: Intercept panggilan fungsi (pre-hook, post-hook, replacement)
2. **Memory Reading/Writing**: Monitor dan modifikasi nilai variabel runtime
3. **Network Interception**: MITM traffic via proxy (mitmproxy, Burp)
4. **Syscall Tracing**: Trace system calls (strace, Frida)
5. **Thread Analysis**: Monitor thread behavior dan synchronization

### Referensi
- Ole André Vadla Ravnås et al., "Frida: Dynamic instrumentation toolkit," https://frida.re/
- rovo89, "Xposed Framework," https://github.com/rovo89/Xposed
- sensepost, "Objection: Runtime mobile exploration," https://github.com/sensepost/objection

---

## Phase 4: Vulnerability Research & Fuzzing

### Tujuan
Menemukan vulnerability melalui analisis data flow, fuzzing, dan exploit development.

### Alat Fuzzing

| Tool | Tipe | Mekanisme |
|------|------|-----------|
| **AFL/AFL++** | Coverage-guided fuzzer | Mutation-based, edge coverage |
| **libFuzzer** | In-process fuzzer | LLVM-based, corpus management |
| **Honggfuzz** | Security-oriented fuzzer | Multi-process, hardware counters |
| **Peach** | Grammar-based fuzzer | Protocol-aware, model-based |
| **Peach Max** | Enterprise fuzzer | Commercial, advanced analysis |

### Metode Vulnerability Research

1. **Taint Analysis**: Melacak data dari source → sink
2. **Symbolic Execution**: Explore semua path secara simbolis
3. **Fuzzing**: Generate input acak berdasarkan coverage feedback
4. **Heap Analysis**: Analisis alokasi/de-alokasi memory
5. **Stack Analysis**: Identifikasi buffer overflow, ROP gadgets

### Referensi
- Google Project AFL, "American Fuzzy Lop: Coverage-guided fuzzer," https://github.com/google/AFL
- LLVM, "libFuzzer – A library for coverage-guided fuzz testing," https://llvm.org/docs/LibFuzzer.html
- google/honggfuzz, "Security oriented fuzzer with versatile analysis," https://github.com/google/honggfuzz

---

## Phase 5: Root Access & Kernel Patching

### Tujuan
Mendapatkan akses root untuk analisis mendalam, bypass pertahanan, dan implementasi modifikasi level kernel.

### Pendekatan Root Access

| Level | Metode | Tools |
|-------|--------|-------|
| **Kernel-level** | Kernel Patch Module (KPM) | SukiSU-Ultra, KernelSU, KernelPatch |
| **Boot image** | Boot partition patching | Magisk, KernelSU |
| **Framework-level** | Zygote injection | Zygisk, Shamiko |
| **Userspace** | su binary + management | SuperSU (legacy), libsu |

### Kapan Root Dibutuhkan?

- **Analisis**: Bypass root detection, akses protected files, memory debugging
- **Hooking sistem-wide**: Xposed modules membutuhkan root/Zygisk
- **Modifikasi sistem**: System partition changes, property modification
- **Mod Menu**: Persistent hooks via Xposed/Zygisk membutuhkan root

### Root vs Non-Root Analysis

| Aspek | Tanpa Root | Dengan Root |
|-------|-----------|-------------|
| Hooking | Frida (per-app) | Xposed (system-wide) |
| File access | App sandbox | Full filesystem |
| Memory | Terbatas | `/proc/self/mem` access |
| Network | Proxy only | iptables, raw sockets |
| Persistence | Per-session | Boot-persistent |

### Referensi
- ShirkNeko, "SukiSU-Ultra: Kernel-based Android Root Solution & KPM," https://github.com/ShirkNeko/SukiSU-Ultra
- topjohnwu, "Magisk: The Magic Mount for Android," https://github.com/topjohnwu/Magisk
- bmax121, "KernelPatch," https://github.com/bmax121/KernelPatch

---

## Phase 6: Stealth, Evasion & Anti-Analysis

### Tujuan
Menghindari deteksi dari anti-cheat, anti-tampering, dan root checker saat melakukan analisis atau implementasi mod.

### Teknik Evasion

| Teknik | Target | Metode |
|--------|--------|--------|
| **Root hiding** | Root checker | SukiSU (built-in SUSFS), Magisk Hide, Shamiko |
| **Magisk Hide** | Per-app root deny | Sembunyikan root dari package tertentu |
| **Zygisk** | System-level detection | Inject tanpa modify system partition |
| **Play Integrity Fix** | Google attestation | Spoof device integrity verdict |
| **TitanHider** | Process hiding | Sembunyikan dari /proc scanning |
| **HideMyApplist** | App list hiding | Binder-based app list filtering |

### Referensi
- simonpunk, "susfs4ksu: Root hiding kernel patches," https://gitlab.com/simonpunk/susfs4ksu
- osm0sis, "Play Integrity Fork," https://github.com/osm0sis/PlayIntegrityFork
- Dr-TSNG, "Hide-My-Applist: Xposed module to intercept applist detections," https://github.com/Dr-TSNG/Hide-My-Applist

---

## Phase 7: Implementation — Mod Menu & Hook Code Generation

### Tujuan
Mengimplementasikan modifikasi berdasarkan hasil analisis: membuat hook code, mod menu UI, dan mengintegrasikan dengan root access.

### Arsitektur Mod Menu

```
┌─────────────────────────────────────────────────────────────────┐
│                     ANDROID MOD MENU ARCHITECTURE                │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ┌──────────────────┐    ┌──────────────────┐                  │
│  │  Static Analysis  │    │  Dynamic Analysis │                  │
│  │  (Find Functions) │    │  (Verify Behavior)│                  │
│  └────────┬─────────┘    └────────┬─────────┘                  │
│           │                       │                             │
│           └───────────┬───────────┘                             │
│                       ▼                                         │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │              HOOK CODE GENERATION                         │  │
│  │  • Frida scripts (JavaScript)                            │  │
│  │  • Xposed modules (Java/Kotlin)                          │  │
│  │  • Native hooks (C/C++ via JNI)                          │  │
│  └──────────────────────────┬───────────────────────────────┘  │
│                             ▼                                   │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │              MOD MENU UI                                  │  │
│  │  • Floating overlay window                               │  │
│  │  • Switches, SeekBars, Buttons                           │  │
│  │  • Toggle health, ammo, gold, etc.                       │  │
│  └──────────────────────────┬───────────────────────────────┘  │
│                             ▼                                   │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │              ROOT INTEGRATION                             │  │
│  │  • Xposed/Zygisk (persistent hooks)                      │  │
│  │  • Magisk modules                                        │  │
│  │  • KernelSU / SukiSU (kernel-level)                      │  │
│  └──────────────────────────┬───────────────────────────────┘  │
│                             ▼                                   │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │              STEALTH / EVASION                            │  │
│  │  • Root hiding (SUSFS, Shamiko)                          │  │
│  │  • Anti-debug bypass                                     │  │
│  │  • Anti-tampering bypass                                 │  │
│  └──────────────────────────────────────────────────────────┘  │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### Target Fungsi dalam Game

| Kategori | Contoh Fungsi | Tipe Modifikasi |
|----------|---------------|-----------------|
| **Combat** | `TakeDamage()`, `ApplyDamage()`, `GetHealth()` | God mode, one-hit kill |
| **Inventory** | `GetAmmo()`, `SetAmmo()`, `AddItem()` | Unlimited ammo, item injection |
| **Currency** | `GetGold()`, `AddGold()`, `SpendGold()` | Infinite money |
| **Movement** | `GetSpeed()`, `SetSpeed()`, `Jump()` | Speed hack, fly hack |
| **Visibility** | `IsVisible()`, `SetFOV()`, `RenderDistance()` | Wallhack, ESP |
| **Anti-cheat** | `ReportCheating()`, `ValidateIntegrity()` | Bypass detection |

### Referensi
- LGLTeam, "Android-Mod-Menu: Mod menu for Android games," https://github.com/LGLTeam/Android-Mod-Menu
- maarsalien, "Frida Android Mod Menu: Frida-based mod menu," https://github.com/maarsalien/frida-android-mod-menu
- vfsfitvnm, "frida-il2cpp-bridge: Hook IL2CPP games with Frida," https://github.com/vfsfitvnm/frida-il2cpp-bridge

---

## Phase 8: Output, Packaging & Distribution

### Tujuan
Membangun APK yang sudah dimodifikasi, menandatangani, dan mendistribusikan.

### Workflow Packaging

```
Modified APK
    │
    ├── [apktool b] → Rebuild APK dari smali
    ├── [sign] → Sign dengan debug/release key
    ├── [zipalign] → Optimize alignment
    └── [Test] → Install & verifikasi di device/emulator
```

### Tools Packaging

| Tool | Fungsi |
|------|--------|
| **apktool** | Rebuild APK dari decoded directory |
| **apksigner** | Sign APK dengan keystore |
| **zipalign** | Optimize APK alignment untuk performa |
| **keytool** | Generate/manage signing keys |
| **uber-apk-signer** | Batch signing tool |

### Referensi
- Android Open Source Project, "Signing Your App," https://developer.android.com/studio/publish/app-signing
- patrickfav, "uber-apk-signer: A tool to zipalign, sign and verify Android APKs," https://github.com/patrickfav/uber-apk-signer

---

# Part II — Referensi Tools & Istilah Lengkap

## 9. APK Extraction Tools

### Apktool
Decompiles Android APK files ke smali bytecode dan decoded resources. Memungkinkan modifikasi dan rebuild APK.

```bash
# Decompile
apktool d target.apk -o target_decoded/

# Rebuild
apktool b target_decoded/ -o target_modified.apk
```

**Situs:** https://github.com/iBotPeaches/Apktool

### dex2jar
Mengkonversi Android DEX bytecode ke Java JAR files untuk analisis dengan decompiler Java.

```bash
d2j-dex2jar.sh target.apk -o target-dex2jar.jar
```

**Situs:** https://github.com/pxb1988/dex2jar

### JADX
Decompiler DEX/APK langsung ke Java source code. Lebih modern dari dex2jar+JD-GUI.

```bash
# CLI mode
jadx -d output/ target.apk

# GUI mode
jadx-gui target.apk
```

**Situs:** https://github.com/skylot/jadx

### AXMLPrinter
Mengkonversi Android binary XML (AXML) ke teks XML yang readable. Berguna untuk membaca AndroidManifest.xml yang sudah di-compile.

**Situs:** https://code.google.com/archive/p/android4me/

### apk-parser
Library Python untuk parse APK tanpa decompile penuh. Cepat untuk ekstrak metadata.

**Situs:** https://github.com/npexception/apk-parser

---

## 10. Static Analysis Tools

### Ghidra
Reverse engineering suite open source dari NSA. Mendukung banyak arsitektur dengan P-code IR.

**Fitur utama:**
- Decompiler C dari binary assembly
- P-code (Intermediate Representation) untuk analisis
- Scripting via Java/Python
- Collaborative RE via shared projects

**Situs:** https://ghidra-sre.org/

### IDA Pro
Industry standard disassembler dan decompiler. Proprietary dengan dukungan arsitektur terluas.

**Fitur utama:**
- Multi-processor disassembly (x86, ARM, MIPS, etc.)
- FLIRT (Fast Library Identification and Recognition Technology)
- Hex-Rays decompiler
- Plugin ecosystem yang luas

**Situs:** https://hex-rays.com/ida-pro/

### radare2 / rizin
Framework reverse engineering open source dengan CLI-first approach.

**Fitur utama:**
- ESIL (Evaluable Strings Intermediate Language) — VM untuk analisis
- Scriptable via rlang (Python, Lua, etc.)
- Banyak backend (capstone, keystone, etc.)
- Rizin: fork radare2 dengan fokus pada stabilitas

```bash
# Analisis binary
r2 -A target.so

# Dump symbols
r2 -q -c "aaa; afl" target.so
```

**Situs:** https://github.com/radareorg/radare2 | https://rizin.re/

### Binary Ninja
Platform RE modern dengan API-first design. IL yang mudah dimanipulasi.

**Situs:** https://binary.ninja/

### Capstone
Framework disassembly multi-arch. Digunakan sebagai backend oleh banyak tools (Ghidra, radare2, Frida).

**Situs:** https://www.capstone-engine.org/

### IL2CppDumper
Ekstrak metadata IL2CPP (Unity) → C# stubs + IDA/Ghidra header. Berguna untuk memahami struktur game Unity.

```bash
Il2CppDumper libil2cpp.so globalmetadata.dat output/
```

**Situs:** https://github.com/Perfare/Il2CppDumper

---

## 11. Dynamic Analysis & Hooking Frameworks

### Frida
Dynamic instrumentation toolkit yang menginject JavaScript ke process target. Cross-platform (Android, iOS, Windows, macOS, Linux).

**Arsitektur:**
```
┌─────────────┐     ┌─────────────┐
│  frida-cli  │────▶│ frida-server │────▶ Target Process
│  (PC)       │ RPC │ (Device)    │     (inject agent)
└─────────────┘     └─────────────┘
```

**Contoh hook sederhana:**
```javascript
Java.perform(function() {
    var targetClass = Java.use("com.target.app.TargetClass");
    targetClass.targetMethod.implementation = function(arg) {
        console.log("[*] Method called with: " + arg);
        return true; // modified return value
    };
});
```

**Situs:** https://frida.re/ | https://github.com/frida/frida

### Xposed Framework
Framework hooking persisten di level ART runtime. Modules berjalan setiap boot.

**Arsitektur:**
- Modifikasi `app_process` saat startup
- Modules register via `IXposedHookLoadPackage`
- Hooks persisten across reboots

**Contoh module:**
```java
public class MyModule implements IXposedHookLoadPackage {
    public void handleLoadPackage(LoadPackageParam lpparam) {
        XposedHelpers.findAndHookMethod(
            "com.target.app.TargetClass",
            lpparam.classLoader,
            "targetMethod",
            int.class,
            new XC_MethodHook() {
                protected void beforeHookedMethod(MethodHookParam param) {
                    param.args[0] = 999; // modify argument
                }
            }
        );
    }
}
```

**Variants:**
| Variant | Status | Platform |
|---------|--------|----------|
| Original Xposed | Legacy | Android 4.x-8.x |
| EdXposed | Maintained | Android 8-13 |
| LSPosed | Active | Android 8-15 (Zygisk) |
| Epic | Experimental | Android 5-11 |

**Situs:** https://github.com/rovo89/Xposed | https://github.com/LSPosed/LSPosed

### Objection
Runtime mobile exploration toolkit. High-level API di atas Frida.

```bash
# Install
pip install objection

# Start exploration
objection -g com.target.app explore

# Bypass SSL pinning
android sslpinning disable

# Root detection bypass
android root disable
```

**Situs:** https://github.com/sensepost/objection

### Cydia Substrate
Code patching platform untuk iOS dan Android. Foundation untuk banyak tools modern.

**Situs:** https://www.saurik.com/substrate/

### Androguard
Framework analisis Android (static + dynamic). Python-based.

```python
from androguard.misc import AnalyzeAPK

d, dx, a = AnalyzeAPK("target.apk")
for method in dx.get_methods():
    print(method.name)
```

**Situs:** https://github.com/androguard/androguard

---

## 12. Fuzzing Engines

### AFL / AFL++ (American Fuzzy Lop)
Coverage-guided fuzzer yang menggunakan mutation-based input generation.

**Mekanisme:**
1. Seed input → mutasi (bit flip, byte insert, splice)
2. Jalankan target dengan input
3. Monitor edge coverage (bitmap)
4. Jika coverage baru → simpan sebagai seed baru
5. Loop

**Kelebihan:** Automatic, high throughput, good coverage
**Kekurangan:** Butuh source code atau instrumented binary

**Situs:** https://github.com/google/AFL | https://github.com/AFLplusplus/AFLplusplus

### libFuzzer
In-process coverage-guided fuzzer dari LLVM. Terintegrasi dengan sanitizer.

```cpp
// Fuzzer target function
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Target function to test
    target_function(data, size);
    return 0;
}
```

**Situs:** https://llvm.org/docs/LibFuzzer.html

### Honggfuzz
Security-oriented fuzzer dengan multi-process support dan hardware performance counter analysis.

**Fitur:**
- Multi-process fuzzing (crash exploration mode)
- Hardware performance counter feedback (Intel PT, BTS)
- Stack-based coverage (unlike edge-based AFL)
- Support untuk Windows, Linux, macOS, Android

**Situs:** https://github.com/google/honggfuzz

### Peach
Grammar-based fuzzer. Mendefinisikan format input via Pit file.

**Situs:** https://github.com/peachfuzzer/peach

### Mutational vs Generational Fuzzing

| Tipe | Input | Contoh |
|------|-------|--------|
| **Mutational** | Mutasi dari seed existing | AFL, libFuzzer, Honggfuzz |
| **Generational** | Generate dari grammar/model | Peach, Dharma |

---

## 13. Network & Traffic Analysis

### mitmproxy
Interactive HTTPS proxy untuk analisis dan manipulasi traffic.

```bash
# Start proxy
mitmproxy -p 8080

# Script for automation
mitmdump -p 8080 -s script.py
```

**Situs:** https://mitmproxy.org/ | https://github.com/mitmproxy/mitmproxy

### Burp Suite
Web vulnerability scanner dan HTTP proxy. Community edition gratis.

**Situs:** https://portswigger.net/burp

### Charles Proxy
HTTP proxy/monitor dengan UI. Populer untuk mobile app debugging.

**Situs:** https://charlesproxy.com/

### Wireshark
Network protocol analyzer. Packet-level inspection.

**Situs:** https://www.wireshark.org/

---

## 14. Memory Forensics

### Volatility
Framework analisis memory dump. Mendukung banyak OS dan format.

```bash
# List processes
volatility -f dump.raw --profile=Win7SP1x64 pslist

# Dump process
volatility -f dump.raw --profile=Win7SP1x64 procdump -p 1234 -D output/
```

**Situs:** https://www.volatilityfoundation.org/ | https://github.com/volatilityfoundation/volatility3

### LiME (Linux Memory Extractor)
Loadable kernel module untuk akuisisi memory di Linux/Android.

**Situs:** https://github.com/504ensicsLabs/LiME

### Andriller
Forensic tool untuk Android. Extraction dari backup, databases, keystore.

**Situs:** https://github.com/Andriller/Andriller-ce

---

## 15. Protobuf Reverse Engineering

### Apa itu Protocol Buffers?
Serialization format dari Google. Digunakan untuk komunikasi network dan penyimpanan data di Android apps.

### Struktur Protobuf

```protobuf
syntax = "proto3";

message PlayerData {
    int32 health = 1;
    int32 armor = 2;
    int32 gold = 3;
    string username = 4;
    repeated Item inventory = 5;
}

message Item {
    int32 id = 1;
    string name = 2;
    int32 quantity = 3;
}
```

### Tools Reverse Engineering Protobuf

| Tool | Fungsi | Situs |
|------|--------|-------|
| **pbtk** | Extract .protos dari APK/binary | https://github.com/marin-m/pbtk |
| **protobuf-inspector** | Analyze protobuf without .proto | https://github.com/antirez/protobuf-inspector |
| **protoc** | Compile/decode protobuf | https://github.com/protocolbuffers/protobuf |
| **protodumper** | Dump protobuf traffic | https://github.com/nccgroup/protodumper |

### Workflow Protobuf RE

```
1. Extract APK → find .so/.dex with protobuf usage
2. Run pbtk on APK → extract .proto files
3. Analyze .proto structures
4. Capture traffic via mitmproxy
5. Decode traffic using extracted .protos
6. Identify modifiable fields (health, gold, etc.)
```

### Referensi
- marin-m, "pbtk: A toolset for reverse engineering and fuzzing Protobuf-based apps," https://github.com/marin-m/pbtk
- Google, "Protocol Buffers: Developer Guide," https://protobuf.dev/programming-guides/proto3/

---

## 16. SQL Hook & Database Manipulation

### SQLite di Android
Android apps menggunakan SQLite untuk penyimpanan lokal. Game menyimpan: player data, save files, leaderboard, items.

### Teknik SQL Hook

#### 1. Hook via Frida
```javascript
Java.perform(function() {
    var SQLiteOpenHelper = Java.use("android.database.sqlite.SQLiteOpenHelper");
    SQLiteOpenHelper.getWritableDatabase.implementation = function() {
        var db = this.getWritableDatabase();
        console.log("[*] Database opened: " + this.getDatabaseName());
        return db;
    };
});
```

#### 2. Hook Native SQLite
```javascript
Interceptor.attach(Module.findExportByName("libsqlite.so", "sqlite3_exec"), {
    onEnter: function(args) {
        var sql = Memory.readUtf8String(args[1]);
        console.log("[SQL] " + sql);
    }
});
```

#### 3. Database Manipulation untuk Game Hacking

| Target | SQL Query | Efek |
|--------|-----------|------|
| Player health | `UPDATE players SET health=9999 WHERE id=1` | God mode |
| Gold/currency | `UPDATE players SET gold=999999 WHERE id=1` | Infinite money |
| Inventory | `INSERT INTO items VALUES(1,'legendary_sword',99)` | Item injection |
| Unlockables | `UPDATE unlocks SET unlocked=1 WHERE item_id=5` | Unlock all |
| Score | `UPDATE leaderboard SET score=999999 WHERE user_id=1` | Cheat score |

### Tools SQL Analysis

| Tool | Fungsi |
|------|--------|
| **sqlite3** | CLI untuk query SQLite database |
| **DB Browser for SQLite** | GUI untuk browse/edit database |
| **Frida** | Hook SQL operations runtime |
| **Room Inspector** | Android Studio tool untuk Room databases |

### Referensi
- SQLite, "SQLite: An SQL database engine," https://www.sqlite.org/
- Google, "Room Persistence Library," https://developer.android.com/training/data-storage/room

---

## 17. Metadata & Symbol Recovery

### Jenis Metadata Android

| Metadata | Lokasi | Fungsi |
|----------|--------|--------|
| `AndroidManifest.xml` | Root APK | Permissions, activities, services |
| `resources.arsc` | Root APK | Compiled resources |
| `classes.dex` | Root APK | Java/Kotlin bytecode |
| `lib/*.so` | Native libs | C/C++ compiled code |
| `globalmetadata.dat` | Unity IL2CPP | Type/method/field metadata |

### Tools Metadata Recovery

| Tool | Target | Output |
|------|--------|--------|
| **Il2CppDumper** | Unity IL2CPP | C# stubs + headers |
| **Cpp2IL** | Unity IL2CPP | Reconstructed source |
| **APKParser** | APK metadata | Manifest, resources |
| **dexlib2** | DEX bytecode | Smali manipulation |

---

# Part III — Konsep Inti Reverse Engineering

## 18. Synthesizer

Synthesizer menghasilkan input baru berdasarkan pemahaman struktur program. Berbeda dengan input random, synthesizer menghasilkan input yang terstruktur.

**Tipe:**
- **Grammar-Based**: Menggunakan BNF/EBNF grammar
- **Byte-Level**: Berdasarkan constraint binary (magic bytes, checksum)
- **Mutational**: Mutation dari input existing

---

## 19. Taint Analysis

Melacak alur data dari **source** (input attacker) ke **sink** (operasi kritis).

```c
// Source: buf ditandai tainted
recv(fd, buf, len, 0);
// Propagasi
memcpy(dst, buf, len);
// Sink: dst yang tainted digunakan sebagai pointer
*(int*)dst = value;  // ← VULNERABILITY
```

**Kegunaan:** Menemukan vulnerability, memahami data flow untuk fuzzing.

---

## 20. Expression Synthesis

Pembangunan ekspresi representatif dari kode assembly.

```asm
; Assembly
mov eax, [rbp-0x10]
imul eax, 2
add eax, 5

; Expression Synthesis
v1 = mem[rbp-0x10]
v2 = v1 × 2
v3 = v2 + 5
```

**Kegunaan:** Simplifikasi kode, pattern matching, deobfuscation.

---

## 21. Memory Segmentation

Analisis layout memori program.

| Segment | Isi | Karakteristik |
|---------|-----|---------------|
| `.text` | Code | Execute-only |
| `.rodata` | Read-only data | String constant |
| `.data` | Initialized global | Global variable |
| `.bss` | Uninitialized global | Zero-initialized |
| Heap | Dynamic allocation | malloc/new |
| Stack | Local variables | Function frame |

---

## 22. Coverage Strategies

Digunakan dalam fuzzing untuk mengukur eksplorasi program.

| Strategi | Metrik | Kelebihan | Kekurangan |
|----------|--------|-----------|------------|
| **Block** | Basic block hit | Sederhana, cepat | Tidak granular |
| **Edge** | Transisi block | Granular | Path explosion |
| **Path** | Entry→exit path | Komprehensif | Sangat expensive |

---

## 23. IR (Intermediate Representation)

Bahasa perantara antara source code dan machine code.

| Jenis | Contoh | Karakteristik |
|-------|--------|---------------|
| SSA | LLVM IR | Variable hanya di-assign sekali |
| Three-Address | GCC GIMPLE | 3 operand |
| P-code | Ghidra | 4-bit operand |
| ESIL | radare2 | Stack-based VM |

**Pipeline:**
```
Source → [Frontend] → IR → [Optimizer] → IR → [Backend] → Machine Code
```

---

## 24. Entropy

Uuran randomitas data. Mendeteksi encryption/compression.

| Entropy | Interpretasi |
|---------|--------------|
| 0.0 - 1.0 | Data kosong/null |
| 1.0 - 4.0 | Data terstruktur (code) |
| 4.0 - 6.0 | Data compress |
| 6.0 - 7.5 | Data terenkripsi |
| 7.5 - 8.0 | Full random |

---

## 25. Packer & Recursive Binary Unpacker

### Packer
Tool yang mengompresi/mentransformasi binary untuk proteksi.

| Kategori | Contoh | Karakteristik |
|----------|--------|---------------|
| Compressor | UPX, ASPack | Compress untuk ukuran |
| Protector | Themida, VMProtect | Enkripsi + anti-debug |
| Virtualizer | Themida VM | Kode dijalankan di VM |
| Obfuscator | Obfuscator-LLVM | Sulitkan analisis |

### Recursive Binary Unpacker
Membongkar binary secara rekursif sampai original payload:

```
Packed (Lapisan 3) → Unpack → Packed (Lapisan 2) → Unpack → Original Binary
```

**Tools:** Unipacker, custom entropy-checking scripts.

---

## 26. Section Carver

Mengekstrak section spesifik dari binary PE/ELF.

```
Section Table:
  .text   → 0x1000  size: 0x5000  (code)
  .rdata  → 0x6000  size: 0x2000  (constants)
  .data   → 0x8000  size: 0x1000  (globals)
```

---

## 27. Compiler Analysis

### Pipeline Compiler
```
Source → Lexical → Parsing → AST → Semantic → IR → Optimize → Codegen → Binary
```

### Pentingnya dalam RE
- **Decompiler** bekerja terbalik: Machine Code → IR → Pseudocode
- **Compiler identification**: Identifikasi dari binary signature
- **Optimization level**: `-O0` vs `-O2` vs `-O3` mempengaruhi output

---

## 28. Stacktrace

Rekaman urutan fungsi yang dipanggil sampai titik tertentu.

```
#0  0x00007f8a12345678 in crash_function ()
#1  0x00007f8a12345abc in caller_function ()
#2  0x00007f8a12345def in main ()
```

**Kegunaan:** Crash analysis, anti-debug detection, vulnerability analysis.

---

# Part IV — Pertahanan & Anti-Analysis

## 29. Anti Debug

Mendeteksi/mencegah debugger terhubung ke process.

**Metode:**
| Metode | Platform | Implementasi |
|--------|----------|--------------|
| API-based | Win/Linux | `IsDebuggerPresent()`, `ptrace(PTRACE_TRACEME)` |
| Timing | All | Bandingkan execution time |
| Hardware BP | All | Cek debug registers (Dr0-Dr3) |
| Exception | Windows | `RaiseException(EXCEPTION_BREAKPOINT)` |
| Process check | All | Cek parent process, loaded modules |

---

## 30. Anti Cheat

Sistem mendeteksi/mencegah kecurangan game online.

| Level | Contoh | Metode |
|-------|--------|--------|
| User-mode | Custom hooks | Memory scanning |
| Kernel-mode | EasyAntiCheat, Vanguard | Driver-level monitoring |
| Hardware | TPM, Secure Boot | Hardware attestation |
| Server-side | Server validation | Behavior analysis |

---

## 31. Anti-Tampering

Mendeteksi modifikasi tidak sah terhadap binary/data.

**Tipe:**
- **Code Integrity**: CRC/Hash checking
- **Data Integrity**: Encrypted variables, checksummed structures
- **Runtime State**: Cross-validation, timing checks

---

## 32. Root Checker

Mendeteksi apakah perangkat Android di-root.

**Metode:**
| Tipe | Implementasi |
|------|--------------|
| File-based | Cek `/system/bin/su`, `/system/xbin/su` |
| Package-based | Cek package Magisk, SuperSU |
| Property-based | Cek `ro.debuggable`, `ro.build.tags` |
| Binary execution | `Runtime.exec("su -c id")` |

---

## 33. SSL Unpinning

Memaksa aplikasi menerima certificate tidak dipinned untuk MITM analysis.

**Metode:**
```javascript
// Frida SSL unpinning
Java.perform(function() {
    var TrustManager = Java.registerClass({
        name: 'com.custom.TrustManager',
        implements: [Java.use('javax.net.ssl.X509TrustManager')],
        methods: {
            checkClientTrusted: function(chain, authType) {},
            checkServerTrusted: function(chain, authType) {},
            getAcceptedIssuers: function() { return []; }
        }
    });
});
```

---

## 34. Play Integrity Check

API Google Play yang memverifikasi integritas perangkat.

**Tingkatan:**
- `INTEGRITY_BASIC`: Verifikasi app legitimate
- `INTEGRITY_DEVICE`: + Verifikasi bootloader locked

---

## 35. Deobfuscation

Mengembalikan kode yang di-obfuscate menjadi readable.

**Tipe:**
| Tipe | Metode |
|------|--------|
| Control Flow | CFG recovery, dominator tree |
| Data Flow | Constant propagation, folding |
| String | Static/dynamic decryption |
| Virtualization | VM opcode recovery |

---

## 36. Hooking Mechanisms

### Perbandingan Metode Hooking

| Metode | Level | Stealth | Kompatibilitas |
|--------|-------|---------|----------------|
| IAT Hooking | User | Medium | Import only |
| Inline/Detour | User | Rendah | Tinggi |
| VTable Hooking | User | Medium | C++ virtual |
| PLT/GOT | User | Medium | Shared lib |
| Inline Trampoline | User | Tinggi | Tinggi |
| Kernel Hooking | Kernel | Tinggi | Perlu driver |

---

## 37. Signature & Magic Number

### File Signature (Magic Bytes)
```
PE:   4D 5A (MZ)
ELF:  7F 45 4C 46
ZIP:  50 4B 03 04
PNG:  89 50 4E 47
APK:  50 4B 03 04 (ZIP-based)
```

### Compiler/Library Signature
```python
strings_indicators = {
    "Microsoft Visual C++": "Microsoft Visual C",
    "GCC": "GCC:",
    "Go": "go.buildid",
    "Rust": ".rustc",
    "Unity": "Unity Engine",
}
```

### Valid Magic Number (Unity IL2CPP)
```
0xFAB11BAF — IL2CPP metadata magic
0x5A4F4F43 — GameMaker data.win
0x7F454C46 — ELF binary
0x4D5A9000 — PE binary
```

---

# Part V — Konsep Lanjutan

## 38. Sanitizer Mechanism

Runtime check untuk mendeteksi vulnerability.

| Sanitizer | Deteksi |
|-----------|---------|
| ASan | Buffer overflow, UAF, double-free |
| MSan | Uninitialized memory read |
| TSan | Data race, deadlocks |
| UBSan | Integer overflow, null deref |
| HWASan | Tag-based memory safety (ARM) |

**Shadow Memory:**
```
8 byte program → 1 byte shadow
Shadow = 0: valid
Shadow > 0: partially/fully invalid
```

---

## 39. Basic Heap Allocator

### Struktur Data
```c
struct chunk_header {
    size_t size;
    chunk_header* fd;  // Forward pointer
    chunk_header* bk;  // Backward pointer
};
```

### Vulnerability Pattern
```c
char* a = malloc(16);
char* b = malloc(16);
// a overflow ke header b → corrupt size, fd, bk
// → free(b) → corrupted free list → RCE
```

### Tcache (glibc 2.26+)
- Per-thread cache untuk alokasi kecil (< 0x410)
- Single linked list
- Lebih mudah di-exploit (tidak ada double-free detection)

---

## 40. Pointer Coverage

Extension dari coverage analysis yang fokus pada pointer values.

**Deteksi:**
- Use-After-Free
- Double-Free
- Buffer Overflow via pointer arithmetic
- Dangling Pointer

---

## 41. Metadata IL2CPP/globalmetadata.dat

### Struktur
```
globalmetadata.dat
├── String Literals
├── Type Definitions
├── Method Definitions
├── Field Definitions
├── Parameter Definitions
├── Generic Containers
├── Image Definitions
└── Usage Hints
```

### Magic Number
```
0xFAB11BAF (little-endian: AF 1B AB FA)
```

### Tools
| Tool | Fungsi |
|------|--------|
| Il2CppDumper | Extract → C# stubs + headers |
| il2cpp-inspector | UI untuk analisis |
| Cpp2IL | Reverse engineer ke source |

### Metadata Versions
| Unity Version | Format |
|---------------|--------|
| Unity 5.x | v14 |
| Unity 2017-2019 | v19-24 |
| Unity 2020+ | v27-29 |
| Unity 2022+ | v29 |

---

# Part VI — Root Access Ecosystem

## 42. SukiSU-Ultra

### Definisi
SukiSU-Ultra adalah **kernel-based Android root solution** yang merupakan fork dari KernelSU, dengan dukungan **Kernel Patch Module (KPM)**. Berbeda dengan Magisk (userspace), SukiSU beroperasi langsung di level kernel.

**GitHub:** https://github.com/ShirkNeko/SukiSU-Ultra

### Arsitektur

```
┌─────────────────────────────────────────────────────────────┐
│                    SukiSU-Ultra Architecture                 │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  ┌───────────────────────────────────────────────────────┐  │
│  │                   KERNEL LEVEL (Ring 0)               │  │
│  │                                                       │  │
│  │  ┌─────────────────┐  ┌─────────────────────────┐   │  │
│  │  │ KernelSU Base   │  │ KPM (Kernel Patch Module)│   │  │
│  │  │ • su binary     │  │ • Custom kernel patches   │   │  │
│  │  │ • Root mgmt     │  │ • SELinux bypass          │   │  │
│  │  │ • Module system │  │ • Mount hiding            │   │  │
│  │  └─────────────────┘  └─────────────────────────┘   │  │
│  │                                                       │  │
│  │  ┌─────────────────────────────────────────────────┐  │  │
│  │  │              SUSFS (Root Hiding)                 │  │  │
│  │  │ • Filesystem manipulation                        │  │  │
│  │  │ • Process hiding                                 │  │  │
│  │  │ • Mount namespace isolation                      │  │  │
│  │  └─────────────────────────────────────────────────┘  │  │
│  └───────────────────────────────────────────────────────┘  │
│                                                             │
│  ┌───────────────────────────────────────────────────────┐  │
│  │                USERSPACE LEVEL (Ring 3)               │  │
│  │                                                       │  │
│  │  ┌─────────────────┐  ┌─────────────────────────┐   │  │
│  │  │ Manager App     │  │ Module System             │   │  │
│  │  │ • Root granting │  │ • Magic Mount             │   │  │
│  │  │ • App profiles  │  │ • Module installation     │   │  │
│  │  │ • SUSFS config  │  │ • Systemless modifications│   │  │
│  │  └─────────────────┘  └─────────────────────────┘   │  │
│  └───────────────────────────────────────────────────────┘  │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

### Mekanisme Kernel Patching

#### 1. SELinux Bypass
```c
// Menulis ke /sys/fs/selinux/enforce → permissive mode
// Memungkinkan akses tanpa batas SELinux
```

#### 2. Mount Hiding
```c
// Unmount path yang mengungkap root:
// - /proc/tty/drivers
// - /proc/net/if_inet6
// - /sys/class/net
// - /proc/self/maps
```

#### 3. UTS Spoofing
```c
// Mengubah kernel release string di /proc/version
// Menyembunyikan kernel custom
```

#### 4. CPU Spoofing
```c
// Modifikasi /proc/cpuinfo
// Menyembunyikan VM/emulator signature
```

#### 5. su Binary Detection Bypass
```
Path yang di-check:
/system/bin/su
/system/xbin/su
/sbin/su
/data/adb/ksu/bin/su
/data/adb/ap/bin/su
/data/adb/su
```

### Fitur Utama

| Fitur | Deskripsi |
|-------|-----------|
| **Kernel-based root** | Root di level kernel, lebih stabil |
| **KPM support** | Kernel Patch Module untuk custom patches |
| **App Profile** | Restrict root per-app |
| **SUSFS built-in** | Root hiding tanpa module terpisah |
| **GKI & non-GKI** | Kompatibel dengan banyak device |
| **Metamodule** | Module mounting delegation |

### Kompatibilitas

| Kernel | Status |
|--------|--------|
| GKI 2.0 (5.10+) | ✅ Official support |
| GKI 1.0 | ✅ Supported |
| Non-GKI (4.4+) | ✅ With manual build |
| Legacy (3.x) | ✅ With backports |

### Referensi
- ShirkNeko, "SukiSU-Ultra," https://github.com/ShirkNeko/SukiSU-Ultra
- XDA Forums, "How to install SukiSu Ultra with SUSFS," https://xdaforums.com/t/how-to-install-sukisu-ultra-with-susfs.4736031/
- magiskzip.com, "SukiSU Ultra: Kernel-Based Android Root Solution & KPM," https://magiskzip.com/sukisu-ultra/
- techkaran.com, "SukiSU Ultra — Kernel-based Android Root Solution," https://www.techkaran.com/2025/11/sukisu-ultra-kernel-based-android-root.html

---

## 43. Libsu

### Definisi
Libsu adalah **Android Java/Kotlin library** yang menyediakan API untuk root operations secara programmatic. Tidak melakukan kernel patching — hanya **menggunakan** root yang sudah tersedia.

### Arsitektur

```
┌─────────────────────────────────────────────────┐
│                  Libsu API                       │
├─────────────────────────────────────────────────┤
│                                                   │
│  ┌─────────────┐  ┌─────────────┐              │
│  │  SuProcess  │  │   SuFile    │              │
│  │  • exec()   │  │  • read()   │              │
│  │  • shell()  │  │  • write()  │              │
│  │  • close()  │  │  • exists() │              │
│  └─────────────┘  └─────────────┘              │
│                                                   │
│  ┌─────────────┐  ┌─────────────┐              │
│  │     Sh      │  │    Job      │              │
│  │  • open()   │  │  • add()    │              │
│  │  • cmd()    │  │  • exec()   │              │
│  │  • close()  │  │  • callback │              │
│  └─────────────┘  └─────────────┘              │
│                                                   │
└─────────────────────────────────────────────────┘
        │
        ▼
┌─────────────────────────────────────────────────┐
│         Root Access Provider                     │
│  (Magisk, KernelSU, SukiSU, SuperSU, etc.)     │
└─────────────────────────────────────────────────┘
```

### Contoh Penggunaan

```kotlin
// Eksekusi command root
Shell.cmd("mount -o remount,rw /system").exec()

// Baca file sebagai root
val suFile = SuFile("/data/data/com.target.app/databases/game.db")
if (suFile.exists()) {
    val content = suFile.readText()
}

// Async root job
Shell.cmd("rm -rf /data/cache/target")
    .callback { result ->
        if (result.isSuccess) {
            // Success
        }
    }
    .submit()
```

### Kapan Libsu Digunakan?

| Skenario | Libsu? | Alternatif |
|----------|--------|------------|
| Root access check | ✅ | `Runtime.exec("su")` |
| File operations as root | ✅ | `su -c cat` |
| Command execution | ✅ | Direct shell |
| Kernel patching | ❌ | KernelSU, SukiSU |
| Hooking | ❌ | Frida, Xposed |

### Referensi
- topjohnwu, "libsu: A library that provides a clean Java API for executing shell commands as root," https://github.com/topjohnwu/libsu

---

## 44. Kernel Driver untuk Android Rooting

### Perbandingan Pendekatan

| Pendekatan | Mekanisme | Contoh |
|------------|-----------|--------|
| **KPM (Kernel Patch Module)** | Patch kernel code langsung | SukiSU, KernelSU |
| **Loadable Kernel Module (.ko)** | Load via insmod/modprobe | KernelPatch |
| **Binder-based** | IPC userspace↔kernel | Magisk |
| **Boot image patching** | Patch boot partition | Magisk, KernelSU |

### Mekanisme Kernel Patching

#### 1. Syscall Table Hooking
```c
// Memodifikasi syscall table untuk intercept panggilan
// Contoh: hook __arm64_sys_openat untuk memfilter file access
```

#### 2. Credential Modification
```c
// Memodifikasi cred struct untuk privilege escalation
// Contoh: ubah uid/gid process menjadi 0 (root)
```

#### 3. SELinux Hooking
```c
// Hook selinux_inode_permission untuk bypass policy
// Memungkinkan akses tanpa batas SELinux
```

### Perbandingan Root Solutions

| Solution | Level | Metode | Kelebihan |
|----------|-------|--------|-----------|
| **Magisk** | Framework | Binder IPC, resetprop | Mature, banyak module |
| **KernelSU** | Kernel | KPM-based | Kernel-level stability |
| **SukiSU-Ultra** | Kernel | KPM + SUSFS | Built-in root hiding |
| **KernelPatch** | Kernel | Generic KPM | Flexible, APatch support |
| **SuperSU** (legacy) | Userspace | su binary | Legacy compatibility |

### Referensi
- Weishu, "KernelSU: A Kernel-based root solution," https://github.com/tiann/KernelSU
- bmax121, "KernelPatch," https://github.com/bmax121/KernelPatch
- m0nad, "Diamorphine: LKM rootkit for Linux," https://github.com/m0nad/Diamorphine

---

## 45. xdl (Extended Dynamic Linker)

### Definisi
xdl adalah **dynamic library loading adapter** yang menyediakan mekanisme untuk symbol resolution, hooking, dan anti-detection pada level dynamic linking.

### Segmentation

```
xdl (Extended Dynamic Linker)
│
├── SEGMENT 1: Symbol Resolution
│   ├── xdl_open()     — Load .so library
│   ├── xdl_close()    — Unload library
│   └── xdl_addr()     — Resolve symbol address
│
├── SEGMENT 2: Hooking
│   ├── xdl_hook()     — Inline/detour hooking
│   ├── xdl_trampoline() — Create trampoline
│   └── xdl_unhook()   — Restore original
│
├── SEGMENT 3: Anti-detection
│   ├── dlopen bypass
│   ├── Symbol hiding
│   └── Memory unmapping
│
└── SEGMENT 4: Process Injection
    ├── ptrace-based
    ├── LD_PRELOAD
    └── Remote dlopen
```

### Metode Hooking xdl

| Metode | Mekanisme | Deteksi Risk |
|--------|-----------|--------------|
| **PLT hooking** | Overwrite Procedure Linkage Table | Medium |
| **GOT hooking** | Overwrite Global Offset Table | Medium |
| **Inline hooking** | Replace first N bytes function | High |
| **Detour hooking** | Jump ke handler + trampoline | High |

### Process Injection

| Metode | Mekanisme | Stealth |
|--------|-----------|---------|
| ptrace-based | Attach ke running process | Medium |
| LD_PRELOAD | Environment variable preload | Low |
| Remote dlopen | Load library di target process | Medium |

### Referensi
-独创的 Extended Dynamic Linker implementation (dalam konteks OmniByte runtime/Evasion)

---

## 46. Varian Root Lainnya

| Varian | Tipe | Mekanisme | Situs |
|--------|------|-----------|-------|
| **Magisk** | Framework | Binder IPC, resetprop, modules | https://github.com/topjohnwu/Magisk |
| **KernelSU** | Kernel | KPM-based, su binary, overlayfs | https://github.com/tiann/KernelSU |
| **KernelPatch** | Kernel | Generic KPM framework | https://github.com/bmax121/KernelPatch |
| **Shamiko** | Zygisk module | Hook Zygote process | https://github.com/LSPosed/LSPosed |
| **PlayIntegrityFix** | Play Store bypass | Spoof device fingerprint | https://github.com/osm0sis/PlayIntegrityFork |
| **Zygisk** | Magisk's injection | Inject via Zygote | Built-in Magisk |
| **HideMyApplist** | App list hiding | Binder-based hiding | https://github.com/Dr-TSNG/Hide-My-Applist |
| **TitanHider** | Process hiding | Hide from /proc | https://github.com/nicehash/TitanHider |
| **APatch** | Kernel | KPM + KernelPatch | https://github.com/bmax121/APatch |

### Hierarki Root Ecosystem

```
Root Access
├── Kernel Level (SukiSU, KernelSU, KernelPatch)
│   ├── Kernel module loading
│   ├── Syscall table hooking
│   └── Credential manipulation
│
├── Framework Level (Magisk, Zygisk)
│   ├── Zygote injection
│   ├── System property modification
│   └── SELinux policy changes
│
└── App Level (libsu, RootBeer, etc.)
    ├── Root detection evasion
    └── Root request handling
```

---

# Part VII — Android Mod Menu

## 47. Mod Menu Architecture

### Definisi
Mod Menu adalah overlay UI yang memungkinkan pengguna mengontrol modifikasi game secara real-time. Biasanya berupa floating window dengan switches, seekbars, dan buttons.

### Arsitektur Komponen

```
┌─────────────────────────────────────────────────────────────────┐
│                     MOD MENU ARCHITECTURE                        │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │                    UI LAYER                               │  │
│  │  • Floating overlay (TYPE_APPLICATION_OVERLAY)           │  │
│  │  • Switches (toggle on/off)                              │  │
│  │  • SeekBars (slider value)                               │  │
│  │  • Buttons (execute action)                              │  │
│  │  • TextViews (display info)                              │  │
│  └──────────────────────────┬───────────────────────────────┘  │
│                             │                                   │
│                             ▼                                   │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │                    HOOK LAYER                             │  │
│  │  • Frida JavaScript hooks                                │  │
│  │  • Xposed module hooks                                   │  │
│  │  • Native JNI hooks                                      │  │
│  │  • IL2CPP bridge hooks                                   │  │
│  └──────────────────────────┬───────────────────────────────┘  │
│                             │                                   │
│                             ▼                                   │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │                    GAME LAYER                             │  │
│  │  • Unity (Mono / IL2CPP)                                 │  │
│  │  • Unreal Engine (Blueprint / C++)                       │  │
│  │  • Native Android (Java / C++)                           │  │
│  │  • Cocos2d-x (JavaScript / C++)                          │  │
│  └──────────────────────────────────────────────────────────┘  │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### Referensi
- LGLTeam, "Android-Mod-Menu," https://github.com/LGLTeam/Android-Mod-Menu
- maarsalien, "Frida Android Mod Menu," https://github.com/maarsalien/frida-android-mod-menu

---

## 48. Hook Code Generation

### Metode Generasi Hook Code

#### 1. Manual Hook (Frida)
```javascript
Java.perform(function() {
    // Target: PlayerController.GetHealth()
    var PlayerController = Java.use("com.game.PlayerController");
    
    // Pre-hook: Intercept sebelum fungsi dieksekusi
    PlayerController.getHealth.implementation = function() {
        console.log("[*] getHealth() called");
        var originalHealth = this.getHealth();
        console.log("[*] Original health: " + originalHealth);
        return 9999; // Return modified value
    };
    
    // Post-hook: Intercept setelah fungsi
    PlayerController.takeDamage.overload('int').implementation = function(damage) {
        console.log("[*] takeDamage(" + damage + ") called");
        // Cancel damage
        this.takeDamage(0);
    };
});
```

#### 2. Xposed Module (Java)
```java
public class GameMod implements IXposedHookLoadPackage {
    @Override
    public void handleLoadPackage(LoadPackageParam lpparam) {
        if (!lpparam.packageName.equals("com.target.game")) return;
        
        // Hook PlayerController.GetHealth()
        XposedHelpers.findAndHookMethod(
            "com.game.PlayerController",
            lpparam.classLoader,
            "getHealth",
            new XC_MethodHook() {
                @Override
                protected void afterHookedMethod(MethodHookParam param) {
                    // Set return value ke 9999
                    param.setResult(9999);
                }
            }
        );
    }
}
```

#### 3. IL2CPP Bridge (Frida)
```javascript
// Menggunakan frida-il2cpp-bridge untuk hook Unity IL2CPP games
const il2cpp = Il2Cpp.perform(() => {
    const PlayerController = Il2Cpp.domain.assembly("Assembly-CSharp")
        .image.class("PlayerController");
    
    // Hook GetHealth
    PlayerController.method("GetHealth").implementation = function() {
        return 9999;
    };
});
```

#### 4. Native Hook (C/C++)
```c
// Hook native function via PLT/GOT
void hook_GetHealth(void* original, void* hook, void** trampoline) {
    // Create trampoline
    *trampoline = mmap(NULL, 12, PROT_READ | PROT_WRITE | PROT_EXEC,
                       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    memcpy(*trampoline, original, 5);
    *(uint8_t*)((uint8_t*)*trampoline + 5) = 0xE9;
    *(uint32_t*)((uint8_t*)*trampoline + 6) = 
        (uint32_t)((uint8_t*)original + 5) - (uint32_t)((uint8_t*)*trampoline + 10);
    
    // Install hook
    mprotect(original, 4096, PROT_READ | PROT_WRITE | PROT_EXEC);
    *(uint8_t*)original = 0xE9;
    *(uint32_t*)((uint8_t*)original + 1) = 
        (uint32_t)hook - (uint32_t)original - 5;
}
```

### Tools Generasi Otomatis

| Tool | Input | Output |
|------|-------|--------|
| **Frida CodeShare** | Function signature | Frida script |
| **hooker** (CreditTone) | Target class/method | Auto-generated hooks |
| **Il2CppDumper** | libil2cpp.so + metadata | C# stubs → hook targets |
| **Ghidra scripts** | Binary analysis | Hook point identification |

### Referensi
- CreditTone, "hooker: Frida-based reverse engineering toolkit," https://github.com/CreditTone/hooker
- vfsfitvnm, "frida-il2cpp-bridge," https://github.com/vfsfitvnm/frida-il2cpp-bridge

---

## 49. Target Fungsi dalam Game

### Kategori Target

| Kategori | Fungsi Target | Modifikasi | Risiko Deteksi |
|----------|---------------|------------|----------------|
| **Health/Combat** | `GetHealth()`, `TakeDamage()`, `ApplyDamage()` | God mode, one-hit kill | Medium |
| **Ammo/Weapon** | `GetAmmo()`, `SetAmmo()`, `Reload()` | Unlimited ammo | Medium |
| **Currency** | `GetGold()`, `AddGold()`, `SpendGold()` | Infinite money | High |
| **Movement** | `GetSpeed()`, `SetSpeed()`, `Jump()` | Speed hack, fly | High |
| **Visibility** | `IsVisible()`, `SetFOV()` | Wallhack, ESP | High |
| **Anti-cheat** | `ReportCheating()`, `ValidateIntegrity()` | Bypass detection | Critical |

### Contoh Hook Target (Unity)

```javascript
// Contoh: Hook fungsi health di Unity game
Java.perform(function() {
    // Method 1: Hook via class name
    var PlayerHealth = Java.use("com.game.PlayerHealth");
    PlayerHealth.getCurrentHealth.implementation = function() {
        return 9999;
    };
    
    // Method 2: Hook via IL2CPP bridge
    // (gunakan frida-il2cpp-bridge untuk games tanpa source)
});
```

### Workflow Identifikasi Target

```
1. Decompile APK (apktool/jadx)
2. Cari class Player/Character/Avatar
3. Identifikasi method: get/set Health, Ammo, Gold
4. Verifikasi via dynamic analysis (Frida)
5. Generate hook code
6. Implementasi di mod menu
```

---

## 50. Mod Menu Implementation Flow

### Workflow Lengkap

```
┌─────────────────────────────────────────────────────────────────┐
│              MOD MENU IMPLEMENTATION WORKFLOW                     │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  PHASE 1: ANALYSIS                                              │
│  ├── Extract APK (apktool, jadx)                               │
│  ├── Static analysis → find target functions                    │
│  ├── Protobuf analysis → identify network messages              │
│  ├── SQL analysis → identify database schema                    │
│  └── Metadata analysis → IL2CPP types/methods                   │
│                                                                 │
│  PHASE 2: VERIFICATION                                          │
│  ├── Dynamic analysis (Frida) → verify function behavior        │
│  ├── Hook function → confirm it affects gameplay                │
│  ├── Memory scanning → find alternative addresses               │
│  └── Network capture → understand protocol                      │
│                                                                 │
│  PHASE 3: IMPLEMENTATION                                        │
│  ├── Generate hook code (Frida/Xposed/Native)                  │
│  ├── Build mod menu UI (floating overlay)                       │
│  ├── Integrate hooks with UI controls                           │
│  └── Add root integration (Xposed/Zygisk module)               │
│                                                                 │
│  PHASE 4: STEALTH                                               │
│  ├── Bypass root detection (SUSFS, Shamiko)                    │
│  ├── Bypass anti-debug                                          │
│  ├── Bypass anti-tampering                                      │
│  ├── Bypass SSL pinning (if needed)                             │
│  └── Hide mod menu from game detection                          │
│                                                                 │
│  PHASE 5: TESTING & DISTRIBUTION                                │
│  ├── Test on device/emulator                                    │
│  ├── Verify all features work                                   │
│  ├── Package as Xposed module / Magisk module                   │
│  ├── Sign APK                                                   │
│  └── Distribute                                                 │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### Contoh: Mod Menu untuk Unity IL2CPP Game

```javascript
// Frida script untuk mod menu Unity IL2CPP game
Java.perform(function() {
    console.log("[*] Mod Menu loaded");
    
    // Load IL2CPP bridge
    var il2cpp = Il2Cpp.perform(() => {
        // Find assembly
        var assembly = Il2Cpp.domain.assembly("Assembly-CSharp");
        var image = assembly.image;
        
        // Find PlayerController class
        var PlayerController = image.class("PlayerController");
        
        // Hook GetHealth
        PlayerController.method("GetHealth").implementation = function() {
            if (modMenu.healthEnabled) {
                return modMenu.healthValue;
            }
            return this.method("GetHealth").invoke();
        };
        
        // Hook GetAmmo
        PlayerController.method("GetAmmo").implementation = function() {
            if (modMenu.ammoEnabled) {
                return modMenu.ammoValue;
            }
            return this.method("GetAmmo").invoke();
        };
        
        // Hook GetGold
        PlayerController.method("GetGold").implementation = function() {
            if (modMenu.goldEnabled) {
                return modMenu.goldValue;
            }
            return this.method("GetGold").invoke();
        };
    });
    
    console.log("[*] Hooks installed");
});
```

### Contoh: Mod Menu UI (Android Overlay)

```java
// Floating mod menu window
public class ModMenuService extends Service {
    private WindowManager windowManager;
    private View modMenuView;
    
    @Override
    public void onCreate() {
        super.onCreate();
        
        windowManager = (WindowManager) getSystemService(WINDOW_SERVICE);
        
        // Create floating view
        modMenuView = LayoutInflater.from(this).inflate(R.layout.mod_menu, null);
        
        // Configure window params
        WindowManager.LayoutParams params = new WindowManager.LayoutParams(
            WindowManager.LayoutParams.WRAP_CONTENT,
            WindowManager.LayoutParams.WRAP_CONTENT,
            WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY,
            WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE,
            PixelFormat.TRANSLUCENT
        );
        
        // Add toggle buttons
        Switch healthToggle = modMenuView.findViewById(R.id.health_toggle);
        healthToggle.setOnCheckedChangeListener((buttonView, isChecked) -> {
            ModConfig.healthEnabled = isChecked;
        });
        
        windowManager.addView(modMenuView, params);
    }
}
```

---

## 51. Kesimpulan & Relevansi untuk OmniByte

### Workflow RE Lengkap (Updated)

```
┌─────────────────────────────────────────────────────────────────┐
│                    WORKFLOW REVERSE ENGINEERING                 │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  1. RECONNAISSANCE & EXTRACTION                                │
│     ├── Apktool, dex2jar, jadx → decompile APK                │
│     ├── AXMLPrinter → decode AndroidManifest.xml               │
│     ├── binwalk, file → identify magic bytes                   │
│     └── Section Carver → extract specific sections             │
│                                                                 │
│  2. STATIC ANALYSIS                                             │
│     ├── Ghidra, IDA Pro, radare2 → disassemble/decompile      │
│     ├── IL2CppDumper → extract IL2CPP metadata                 │
│     ├── Expression Synthesis → simplify complex code           │
│     ├── Protobuf analysis → find .proto definitions            │
│     └── SQL analysis → identify database schema                │
│                                                                 │
│  3. DYNAMIC ANALYSIS                                            │
│     ├── Frida → runtime hooking & instrumentation              │
│     ├── Xposed → persistent system-wide hooks                  │
│     ├── mitmproxy → network traffic interception               │
│     └── Memory forensics → runtime data analysis               │
│                                                                 │
│  4. VULNERABILITY RESEARCH                                      │
│     ├── Fuzzing (AFL, libFuzzer) → input generation           │
│     ├── Taint analysis → data flow tracking                    │
│     ├── Heap analysis → memory corruption                      │
│     └── Protobuf fuzzing → protocol manipulation               │
│                                                                 │
│  5. ROOT ACCESS & KERNEL PATCHING                               │
│     ├── SukiSU-Ultra, KernelSU → kernel-level root             │
│     ├── Magisk, Zygisk → framework-level root                  │
│     ├── SUSFS, Shamiko → root hiding                           │
│     └── Play Integrity Fix → attestation bypass                │
│                                                                 │
│  6. STEALTH & EVASION                                           │
│     ├── Anti-debug bypass (timing, API, hardware)              │
│     ├── Anti-tampering bypass (patching, hooking)              │
│     ├── Root hiding (per-app deny, filesystem)                 │
│     └── Process/module hiding                                  │
│                                                                 │
│  7. IMPLEMENTATION (MOD MENU)                                   │
│     ├── Hook code generation (Frida, Xposed, Native)          │
│     ├── Mod menu UI (floating overlay)                         │
│     ├── Target function modification                           │
│     ├── SQL hook (database manipulation)                       │
│     └── Protobuf manipulation (network messages)               │
│                                                                 │
│  8. OUTPUT & DISTRIBUTION                                       │
│     ├── APK rebuild (apktool)                                  │
│     ├── Signing (apksigner, keytool)                           │
│     ├── Module packaging (Xposed, Magisk)                      │
│     └── Distribution                                            │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### Relevansi untuk OmniByte

Pemahaman terhadap workflow ini penting untuk pengembangan OmniByte:

#### Analisis Binary
- **Entropy & Signature** → Deteksi format dan proteksi binary
- **Magic Number (0xFAB11BAF)** → Validasi format file IL2CPP
- **Section Carver & Recursive Unpacker** → Ekstrak kode dari binary terproteksi
- **IR & Compiler** → Memahami bagaimana kode dieksekusi

#### Game Engine Analysis
- **Metadata (IL2CPP)** → Extract type/method dari Unity games
- **Protobuf analysis** → Memahami protocol komunikasi game
- **SQL hook** → Manipulasi database game
- **Hooking** → Intercept fungsi game untuk modifikasi

#### Android Mod Menu Development
- **Hook code generation** → Auto-generate scripts untuk target functions
- **Mod menu UI** → Floating overlay implementation
- **Root integration** → Xposed/Zygisk/Magisk module
- **Stealth/Evasion** → Bypass detection mechanisms

#### Security Research
- **Deobfuscation** → Membongkar kode yang dilindungi
- **Taint Analysis** → Menemukan vulnerability
- **Fuzzing** → Automated vulnerability discovery
- **Anti-Tampering Bypass** → Circumvent integrity checks

#### Tools Development
- **Frida scripts** → Dynamic instrumentation
- **Xposed modules** → Persistent hooks
- **Native hooks** → Low-level interception
- **Root access** → System-level modifications

---

## 52. Daftar Pustaka & Sitasi

### Root Access & Kernel Patching
1. ShirkNeko, "SukiSU-Ultra: Kernel-based Android Root Solution & KPM," GitHub, https://github.com/ShirkNeko/SukiSU-Ultra
2. topjohnwu, "Magisk: The Magic Mount for Android," GitHub, https://github.com/topjohnwu/Magisk
3. tiann, "KernelSU: A Kernel-based root solution," GitHub, https://github.com/tiann/KernelSU
4. bmax121, "KernelPatch," GitHub, https://github.com/bmax121/KernelPatch
5. simonpunk, "susfs4ksu: Root hiding kernel patches," GitLab, https://gitlab.com/simonpunk/susfs4ksu
6. osm0sis, "Play Integrity Fork," GitHub, https://github.com/osm0sis/PlayIntegrityFork
7. Dr-TSNG, "Hide-My-Applist: Xposed module to intercept applist detections," GitHub, https://github.com/Dr-TSNG/Hide-My-Applist
8. XDA Forums, "How to install SukiSu Ultra with SUSFS," https://xdaforums.com/t/how-to-install-sukisu-ultra-with-susfs.4736031/
9. magiskzip.com, "SukiSU Ultra: Kernel-Based Android Root Solution & KPM," https://magiskzip.com/sukisu-ultra/
10. techkaran.com, "SukiSU Ultra — Kernel-based Android Root Solution," https://www.techkaran.com/2025/11/sukisu-ultra-kernel-based-android-root.html

### APK Extraction & Decompilation
11. iBotPeaches, "Apktool: A tool for reverse engineering Android APK files," GitHub, https://github.com/iBotPeaches/Apktool
12. skylot, "JADX - Dex to Java decompiler," GitHub, https://github.com/skylot/jadx
13. pxb1988, "dex2jar: Tools to work with android .dex and java .class files," GitHub, https://github.com/pxb1988/dex2jar

### Static Analysis
14. National Security Agency, "Ghidra: A Software Reverse Engineering Framework," https://ghidra-sre.org/
15. Hex-Rays, "IDA Pro: Multi-processor disassembler and debugger," https://hex-rays.com/ida-pro/
16. radareorg, "radare2: UNIX-like reverse engineering framework," GitHub, https://github.com/radareorg/radare2
17. rizin, "rizin: Free and open-source reverse engineering framework," https://rizin.re/
18. Capstone Engine, "Capstone: Disassembly framework for multi-arch," https://www.capstone-engine.org/

### Dynamic Analysis & Hooking
19. Ole André Vadla Ravnås et al., "Frida: Dynamic instrumentation toolkit," https://frida.re/
20. rovo89, "Xposed Framework," GitHub, https://github.com/rovo89/Xposed
21. LSPosed, "LSPosed: Xposed framework for Android 8+," GitHub, https://github.com/LSPosed/LSPosed
22. sensepost, "Objection: Runtime mobile exploration," GitHub, https://github.com/sensepost/objection
23. CreditTone, "hooker: Frida-based reverse engineering toolkit," GitHub, https://github.com/CreditTone/hooker

### Fuzzing
24. Google, "AFL: American Fuzzy Lop: Coverage-guided fuzzer," GitHub, https://github.com/google/AFL
25. AFLplusplus, "AFL++: Enhanced American Fuzzy Lop," GitHub, https://github.com/AFLplusplus/AFLplusplus
26. LLVM, "libFuzzer: A library for coverage-guided fuzz testing," https://llvm.org/docs/LibFuzzer.html
27. google/honggfuzz, "Honggfuzz: Security oriented fuzzer," GitHub, https://github.com/google/honggfuzz

### Protobuf Analysis
28. marin-m, "pbtk: A toolset for reverse engineering and fuzzing Protobuf-based apps," GitHub, https://github.com/marin-m/pbtk
29. antirez, "protobuf-inspector: Tool to decode protobuf without .proto," GitHub, https://github.com/antirez/protobuf-inspector
30. Google, "Protocol Buffers: Developer Guide," https://protobuf.dev/programming-guides/proto3/

### Mod Menu & Hooking
31. LGLTeam, "Android-Mod-Menu: Mod menu for Android games," GitHub, https://github.com/LGLTeam/Android-Mod-Menu
32. maarsalien, "Frida Android Mod Menu: Frida-based mod menu," GitHub, https://github.com/maarsalien/frida-android-mod-menu
33. vfsfitvnm, "frida-il2cpp-bridge: Hook IL2CPP games with Frida," GitHub, https://github.com/vfsfitvnm/frida-il2cpp-bridge

### Metadata & Symbol Recovery
34. Perfare, "Il2CppDumper: Extract IL2CPP metadata," GitHub, https://github.com/Perfare/Il2CppDumper
35. Android Open Source Project, "Android Binary XML Format," https://android.googlesource.com/platform/frameworks/base/+/master/libs/androidfw/README

### Network & Traffic Analysis
36. mitmproxy, "mitmproxy: Interactive HTTPS proxy," https://mitmproxy.org/
37. PortSwigger, "Burp Suite: Web vulnerability scanner," https://portswigger.net/burp
38. Wireshark, "Wireshark: Network protocol analyzer," https://www.wireshark.org/

### Memory Forensics
39. Volatility Foundation, "Volatility: Memory forensics framework," https://www.volatilityfoundation.org/
40. 504ensicsLabs, "LiME: Linux Memory Extractor," GitHub, https://github.com/504ensicsLabs/LiME

### Database & SQL
41. SQLite, "SQLite: An SQL database engine," https://www.sqlite.org/
42. Google, "Room Persistence Library," https://developer.android.com/training/data-storage/room

### Root Detection & Evasion
43. 5ec1cff, "MKSU: KernelSU with Magic Mount," GitHub, https://github.com/5ec1cff/KernelSU
44. brevent, "genuine: APK v2 signature validation," GitHub, https://github.com/brevent/genuine/

### Documentation & Standards
45. Android Open Source Project, "Signing Your App," https://developer.android.com/studio/publish/app-signing
46. patrickfav, "uber-apk-signer: A tool to zipalign, sign and verify Android APKs," GitHub, https://github.com/patrickfav/uber-apk-signer

---

**Dokumen ini merupakan bagian dari proyek Pengembangan OmniByte dan disusun sebagai referensi teknis untuk tim pengembang.**

**Terakhir diperbarui:** 2026-09-05
**Revisi:** 4.0
