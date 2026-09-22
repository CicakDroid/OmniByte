<p align="center">
  <img src="assets/icon.svg" width="128" alt="OmniByte Icon"/>
</p>

<h1 align="center">OmniByte</h1>

<p align="center">
  <strong>Ferramenta de Engenharia Reversa para Android</strong><br/>
  Descompilador &bull; Editor &bull; Dumper &bull; Hooking &bull; Editor de Memoria &bull; Monitoramento de Rede
</p>

<p align="center">
  <a href="README.md">🇮🇩 Bahasa Indonesia</a> &bull;
  <a href="README_EN.md">🇬🇧 English</a> &bull;
  <strong>🇧🇷 Portugu&ecirc;s</strong> &bull;
  <a href="README_RU.md">🇷🇺 Русский</a> &bull;
  <a href="README_ZH.md">🇨🇳 中文</a>
</p>

---

## Sobre

OmniByte is an Android reverse engineering toolkit built on **Kotlin + C++ Native**, supporting multi-architecture (**ARMv7** & **ARMv8a**) and multi-version Android (**6.0 to latest**).

Designed for static & dynamic analysis, decompilation, binary editing, function hooking, memory editing, and network monitoring, capturing & editing — all in one unified platform. **No LLVM lifting**.

**Special Features:**
- ✅ **Runs with root AND without root** — Supports root access (KernelSU, Magisk, SukiSU) and non-root mode via `/proc/pid/mem`
- 🔍 **Universal Dumper for Android Native Libraries** — Dumps all `.so` libraries from Android processes (libil2cpp.so, libtamarin.so, libunity.so, etc.)
- ⚡ **HPT Orchestrator** — Orchestrates hooking & memory editing through Hooking/MemoryEditing subsystems
- 🔄 **Taskflow Adapter** — Taskflow adaptation for scheduling & parallelization
- 🛡️ **ZigZag Stealth** — Bypass anti-tamper (Pairip, CRC, LRFP/Bypasser, Diamorphine)
- 📤 **Export Pipeline** — Multi-format output (C#, JSON, Header, DummyDll) via ExportCore orchestrator

## Principais Recursos

| Feature | Description |
|---------|-------------|
| 🔍 **APK Decompiler** | Decompile APK to source code (Java/Smali/DEX) with multi-engine support |
| ✏️ **APK Editor** | View & manipulate manifest, resources, smali, and rebuild APK |
| 📊 **Binary Analysis** | Static & dynamic binary analysis (ELF/PE) with disassembler & decompiler |
| 🔧 **Binary Editor** | View, explore & manipulate binaries directly with hex editor & patching |
| 📦 **Universal Dumper** | Dump Android native libraries (7 engines) manually or automatically during Live PID |
| 🪝 **Hooking** | Hook native functions & ART methods with 6 backends (Albatross, Bhook, Vector, KittyMemory, SQLhook, NPhook) |
| 🧠 **Memory Editor** | Read & write live process memory via KittyMemory/KittyMemoryEx |
| 🗄️ **Database Hooking** | Hook SQLite database functions via SQLhook |
| 🌐 **Network Hooking** | Hook libc network functions via NPhook (dlsym + inline hook) |
| 📡 **Network Monitoring** | Monitor, capture, and edit network packets in real-time |
| 🛡️ **ZigZag Stealth** | Bypass anti-tamper & anti-debug (Pairip, CRC, LRFP, Diamorphine) |
| 🔄 **Updater** | Auto-update via GitHub Releases (semver, SHA256, backup/rollback) |

## Suporte a Plataforma

| Component | Support |
|-----------|---------|
| **Android** | 6.0 (API 23) — latest version |
| **Architecture** | ARMv7 (32-bit), ARMv8a (64-bit) |
| **Languages** | Kotlin (UI), C++17 (Native Core) |
| **Build System** | Gradle + CMake |
| **STL** | c++_shared |

## Project Structure

<details open>
<summary><strong>Directory Structure (click to hide/unhide)</strong></summary>

```
OmniByte/
├── app/                                    # Android Application (Kotlin)
│   └── src/main/
│       ├── java/com/omnibyte/app/          # Kotlin Source
│       ├── cpp/                            # Native aggregator (CMake)
│       └── res/                            # Android Resources
│
├── Hydra/                                  # Static & Dynamic Analysis Engine
│   ├── Hydra2D/                            # Core analysis engine
│   │   ├── Disassembler/backends/          # Capstone adapter
│   │   ├── Decompiler/backends/            # Rizin native, rz-ghidra adapter
│   │   ├── Parser/backends/                # LIEF adapter
│   │   ├── Orchestrator/                   # Analysis orchestration
│   │   ├── Factory/                        # Component factory
│   │   ├── Analysis/                       # Binary analysis algorithms (15 classes)
│   │   │   ├── IAnalysis.h                 # Abstract interface + Result structs
│   │   │   ├── Structure, Stack, List, Tree # Core algorithms (trie, CFG, dominator)
│   │   │   ├── Packers, Obfuscate, Taint   # Detection algorithms
│   │   │   ├── Functions, Variables, Params # Naming algorithms
│   │   │   ├── Types, Confidences, Strings # Type & scoring algorithms
│   │   │   └── Imports, Exports            # Symbol algorithms
│   │   ├── Plugin/
│   │   │   ├── Enhanced/                   # Advanced plugins
│   │   │   │   ├── AST/                    # Abstract Syntax Tree
│   │   │   │   ├── CFG/                    # Control Flow Graph
│   │   │   │   ├── Crypt/                  # FindCrypt3, DeCrypt3
│   │   │   │   ├── Deobfuscate/            # DexKit, hrtng
│   │   │   │   ├── Emulation/              # Qemu, Unicorn
│   │   │   │   ├── Renamer/                # Function renaming
│   │   │   │   ├── RTTI/                   # Runtime Type Info
│   │   │   │   ├── Signatures/             # MagicBytes, Pattern, Yara, DB
│   │   │   │   └── SymbolicExecution/      # Triton, Z3, CVC5
│   │   │   └── ScriptHooks/                # Loader + Runner
│   │   └── docs/research-reports/          # Plugin research reports
│   └── Shared/Metadata/                    # Shared metadata (DumpData, entries)
│
├── modules/
│   ├── Dumper/                             # Universal Dumper Module
│   │   ├── DumperCore/                     # Core dumper logic
│   │   │   ├── Detector/                   # Auto engine detection
│   │   │   ├── EngineRegistry/             # Engine registration & lookup
│   │   │   ├── ResultNormalizer/           # Output normalization
│   │   │   ├── SharedUtils/                # Shared utilities
│   │   │   └── WorkingModes/               # Manual & Live PID modes
│   │   ├── Engines/                        # 7 Dumper Engines
│   │   │   ├── UnityIL2CPP/                # Unity IL2CPP (Analyzer, Profiles, Resolver)
│   │   │   ├── UnityMono/                  # Unity Mono (Analyzer, Profiles, Resolver)
│   │   │   ├── UnrealEngine/               # Unreal Engine UE4/UE5 (SignatureBypass, Profiles, Resolver)
│   │   │   ├── Godot/                      # Godot Engine (KeyExtractor, Profiles, Resolver)
│   │   │   ├── Cocos/                      # Cocos2d-x (v2/v3/v4) + Cocos Creator (v1/v2/v3)
│   │   │   ├── GameMaker/                  # GameMaker Studio (Analyzer, Profiles, Resolver)
│   │   │   └── Source2/                    # Valve Source 2 (Analyzer, Profiles, Resolver)
│   │   └── Export/                         # Result export pipeline
│   │       ├── ExportCore/                 # Export orchestrator
│   │       │   ├── ExportRegistry/         # Writer registry (registerDefaults)
│   │       │   ├── IExporter/              # IExporter interface
│   │       │   └── SectionSplitter/        # Section splitter (namespace, type)
│   │       └── Writers/                    # Writer implementations (header-only)
│   │           ├── CSharpWriter/           # .cs output
│   │           ├── JsonWriter/             # .json output
│   │           ├── DummyDllWriter/         # dummy .dll/.so output
│   │           └── HeaderWriter/           # .h output (native header)
│   │
│   ├── HPT/                                # HPT Orchestrator Module
│   │   ├── Hooker/                         # Hooking subsystem (IHookBackend)
│   │   │   └── HookEngines/
│   │   │       ├── ARTHook/Albatross/      # ART method hook
│   │   │       ├── InlineHook/Shadowhook/  # Inline hook
│   │   │       ├── PLT_GOTHook/Bhook/      # PLT/GOT hook
│   │   │       ├── TracelessHook/Vectorhook/ # Traceless hook
│   │   │       ├── DatabaseHook/SQLhook/   # Database hook (sqlite3)
│   │   │       ├── NPHook/NetworkPackethook/ # Network packet hook
│   │   │       └── IoHook/                 # I/O hook
│   │   └── MemoryEditor/                   # Memory editing subsystem (IMemoryEditor)
│   │       └── KittyMemory/                # KittyMemory backend
│   │
│   └── Hooker/                             # Legacy Hooker module
│       └── HookEngine/                     # ARTHook, InlineHook, PLT_GOTHook, TracelessHook
│
├── runtime/                                # Live Device Runtime
│   ├── Bridges/                            # Bridge loaders
│   │   ├── FreedomServiceBridge/           # Root service bridge
│   │   └── ModuleBridge/                   # Module bridge
│   ├── ProcessManager/                     # Process management
│   ├── MemoryIO/                           # Memory read/write (/proc/pid/mem)
│   ├── SymbolResolver/                     # Symbol resolution
│   │   └── xdl-adapter/                    # xdl adapter (dlopen/dlsym)
│   ├── ZigZag/                             # Stealth/bypass engine
│   │   └── backends/
│   │       ├── ApkSignature/               # APK signature bypass
│   │       │   ├── ApkSigKiller/
│   │       │   ├── ApkSigKillerEx/
│   │       │   ├── LoadedApkSpoofer/
│   │       │   ├── NativePmsHook/
│   │       │   ├── SignatureExtractor/
│   │       │   └── SignatureInjector/
│   │       ├── CRC/                        # CRC check bypass
│   │       ├── LRFP/Bypasser/              # LRFP bypass
│   │       ├── Pairip/                     # Pairip bypass
│   │       └── Procees/
│   │           └── Diamorphine/            # Anti-debug
│   ├── ZigZagManager/                      # Stealth lifecycle management
│   ├── HPTManager/                         # HPT lifecycle management
│   └── FreedomService/                     # Root access service
│       ├── KernelSU-Next/                  # KernelSU support
│       ├── SUI/                            # Magisk SUI support
│       ├── SukiSU-Ultra/                   # SukiSU support
│       ├── RootThread/                     # Root thread management
│       └── backends/                       # Root detection backends
│
├── config/                                 # JSON Configuration (ConfigManager)
│   ├── EngineDetection/                    # Engine detection thresholds
│   ├── FileLimits/                         # File size & extension limits
│   ├── Logging/                            # Log level & format
│   ├── Network/                            # Timeout, retry, user-agent
│   ├── Runtime/                            # Backend priority
│   ├── Storage/                            # Output path, backup, auto-cleanup
│   └── UI/                                 # Theme, language, notifications
│
├── common/                                 # Shared Utilities
│   ├── Math/GLM/                           # Math utilities
│   ├── Deserialization-Serialization/Json/ # JSON serialization
│   └── Taskflow/                           # Taskflow adapter (FetchContent v3.8.0)
│
├── updater/                                # Auto-update System
│   ├── GithubReleaseChecker/               # Check new releases (semver, GitHub API)
│   ├── Downloader/                         # Download assets (retry, SHA256)
│   └── Installer/                          # Install updates (backup/rollback)
│
├── toolchain/                              # Build Toolchain
│   ├── rizin-android/                      # Rizin disassembler
│   └── stub-headers/                       # Stub headers (cvc5, rizin, triton, z3)
│
├── engine-core/                            # Legacy engine core
│   └── HydraDis/Plugin/Enhanced/Crypt/
│
├── scripts/                                # Build scripts
├── test/                                   # Test materials
│   └── il2cpp-test/
├── doc/                                    # Documentation
├── docs/                                   # Additional documentation
├── gradle/wrapper/                         # Gradle wrapper
└── assets/                                 # Project assets (icon, etc.)
```

</details>

## Workflow Pipeline

<details open>
<summary><strong>Workflow Pipeline (click to hide/unhide)</strong></summary>

```mermaid
flowchart TB
    subgraph INPUT["Input"]
        APK["APK File"]
        BIN["Binary (ELF/PE)"]
        PID["Live Process (PID)"]
    end

    subgraph HYDRA["Hydra - Static Analysis"]
        direction TB
        H_DIS["Disassembler\n(Capstone / Rizin)"]
        H_DEC["Decompiler\n(Rizin / rz-ghidra)"]
        H_PAR["Parser\n(LIEF)"]
        H_ANA["Analysis Classes\n(15 detection algorithms)"]
        H_PLG["Plugins\n(orchestrators + JSON)"]
        H_ORC["Orchestrator"]
        H_DIS --> H_ORC
        H_DEC --> H_ORC
        H_PAR --> H_ORC
        H_ANA --> H_PLG
        H_PLG --> H_ORC
    end

    subgraph DUMPER["Universal Dumper"]
        direction TB
        D_DET["Detector\n(Auto-detect engine)"]
        D_REG["EngineRegistry\n(Register & Lookup)"]
        D_ENG["Engines\n(UnityIL2CPP, UnityMono,\nUnrealEngine, Godot,\nCocos, GameMaker, Source2)"]
        D_RES["Resolver\n(Symbol, Profile)"]
        D_NORM["ResultNormalizer\n(Output normalization)"]
        D_EXP["ExportCore\n(Orchestrator)"]
        D_WR["Writers\n(CSharp, JSON, Header,\nDummyDll)"]
        D_DET --> D_REG --> D_ENG --> D_RES --> D_NORM --> D_EXP --> D_WR
    end

    subgraph HPT["HPT Orchestrator"]
        direction TB
        HK_MOD["Hooking Subsystem\n(IHookBackend)"]
        ME_MOD["MemoryEditing Subsystem\n(IMemoryEditor)"]
        HK_MOD --> HK_ART["ART Hook\n(Albatross)"]
        HK_MOD --> HK_INL["Inline Hook\n(Shadowhook)"]
        HK_MOD --> HK_PLT["PLT/GOT Hook\n(Bhook)"]
        HK_MOD --> HK_TLS["Traceless Hook\n(Vector)"]
        HK_MOD --> HK_DB["Database Hook\n(SQLhook)"]
        HK_MOD --> HK_NP["Network Hook\n(NPhook)"]
        HK_ART --> HK_MEM["Memory Patching\n(KittyMemory)"]
        HK_INL --> HK_MEM
        HK_PLT --> HK_MEM
        HK_TLS --> HK_MEM
        HK_DB --> HK_MEM
        HK_NP --> HK_MEM
        ME_MOD --> ME_KIT["KittyMemory"]
        ME_MOD --> ME_KITX["KittyMemoryEx"]
    end

    subgraph RUNTIME["Runtime - Live Device"]
        direction TB
        R_BRG["Bridges\n(FreedomService, Module)"]
        R_PM["ProcessManager"]
        R_MIO["MemoryIO"]
        R_SYM["SymbolResolver\n(xdl)"]
        R_ZZ["ZigZag Stealth\n(Pairip, CRC, Bypasser,\nDiamorphine)"]
        R_HPT["HPTManager\n(Lifecycle)"]
        R_FS["FreedomService\n(KernelSU, SUI, SukiSU)"]
        R_BRG --> R_PM
        R_PM --> R_MIO
        R_PM --> R_SYM
        R_ZZ --> R_FS
        R_HPT --> HK_MOD
        R_HPT --> ME_MOD
    end

    subgraph TASKFLOW["Taskflow - Parallel Scheduling"]
        direction TB
        TF["TaskflowAdapter\n(FetchContent v3.8.0)"]
    end

    subgraph OUTPUT["Output"]
        O_CS["dump.cs\n(C# Definitions)"]
        O_ST["struct_dump.cs\n(Struct Layout)"]
        O_SF["static_fields.txt\n(Static Offsets)"]
        O_H["*.h\n(Native Header)"]
        O_JSON["dump.json\n(JSON Structure)"]
        O_SM["Smali Source"]
        O_PT["Patch Result"]
    end

    APK --> H_PAR
    BIN --> H_DIS
    BIN --> D_DET
    PID --> R_PM
    H_ORC --> D_DET
    D_WR --> O_CS
    D_WR --> O_ST
    D_WR --> O_SF
    D_WR --> O_H
    D_WR --> O_JSON
    H_DEC --> O_SM
    HK_MEM --> O_PT
    R_MIO --> HK_MEM
    R_SYM --> D_RES
    TF --> HK_MOD
    TF --> ME_MOD
```

</details>

## Module Architecture

<details open>
<summary><strong>Module Architecture (click to hide/unhide)</strong></summary>

```mermaid
flowchart LR
    subgraph CORE["Core Layer"]
        COMMON["common/\n(Math, Json, Taskflow)"]
        TOOLCHAIN["toolchain/\n(Rizin, Headers)"]
        CFM["config/\n(ConfigManager)"]
    end

    subgraph NATIVE["Native Layer (C++17)"]
        HYDRA["Hydra / Hydra2D\nStatic Analysis"]
        DUMPER["Universal Dumper\nAndroid Native Dumping"]
        HPT["HPT Orchestrator\nHooking + MemoryEditing"]
        RUNTIME["Runtime\nLive Device"]
        UPDATER["Updater\nAuto-Update"]
    end

    subgraph HOOKS["Hooking Backends"]
        HK_ART["Albatross\n(ART Hook)"]
        HK_BHK["Bhook\n(PLT/GOT)"]
        HK_VEC["Vector\n(Traceless)"]
        HK_SHD["Shadowhook\n(Inline Hook)"]
        HK_SQL["SQLhook\n(Database Hook)"]
        HK_NP["NPhook\n(Network Hook)"]
    end

    subgraph DUMP_ENGINES["Dumper Engines"]
        DE_UIL["UnityIL2CPP"]
        DE_UM["UnityMono"]
        DE_UNR["UnrealEngine"]
        DE_GD["Godot"]
        DE_CC["Cocos"]
        DE_GM["GameMaker"]
        DE_S2["Source2"]
    end

    subgraph STEALTH["Stealth Backends"]
        ZZ_PAIR["Pairip"]
        ZZ_CRC["CRC"]
        ZZ_LRFP["LRFP/Bypasser"]
        ZZ_DIAM["Diamorphine"]
        ZZ_APK["ApkSignature"]
    end

    subgraph WRITERS["Export Writers"]
        W_CS["CSharpWriter"]
        W_JSON["JsonWriter"]
        W_DLL["DummyDllWriter"]
        W_HDR["HeaderWriter"]
    end

    COMMON --> HYDRA
    COMMON --> DUMPER
    COMMON --> HPT
    COMMON --> RUNTIME
    TOOLCHAIN --> HYDRA
    CFM --> RUNTIME
    CFM --> DUMPER
    HYDRA --> DUMPER
    DUMPER --> W_CS
    DUMPER --> W_JSON
    DUMPER --> W_DLL
    DUMPER --> W_HDR
    HPT --> HK_ART
    HPT --> HK_BHK
    HPT --> HK_VEC
    HPT --> HK_SHD
    HPT --> HK_SQL
    HPT --> HK_NP
    DUMPER --> DE_UIL
    DUMPER --> DE_UM
    DUMPER --> DE_UNR
    DUMPER --> DE_GD
    DUMPER --> DE_CC
    DUMPER --> DE_GM
    DUMPER --> DE_S2
    RUNTIME --> ZZ_PAIR
    RUNTIME --> ZZ_CRC
    RUNTIME --> ZZ_LRFP
    RUNTIME --> ZZ_DIAM
    RUNTIME --> ZZ_APK
```

</details>

## Build & Installation

<details>
<summary><strong>Build & Installation (click to hide/unhide)</strong></summary>

### Prerequisites
- Android Studio (latest stable)
- Android NDK r29
- CMake 3.22+
- JDK 17+

### Build
```bash
git clone https://github.com/user/OmniByte.git
cd OmniByte
./gradlew assembleDebug
```

### Configuration
All configuration accessed via `ConfigManager::get<T>()`. No direct file hardcoding.

</details>

## Contributing

<details>
<summary><strong>Contributing (click to hide/unhide)</strong></summary>

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit changes (`git commit -m 'Add amazing feature'`)
4. Push to branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

### Rules
- **Comments matter** - All code needs process, document clearly
- **ConfigManager** - Use `ConfigManager::get<T>()` for all config access
- **Multi-threading** - Use `common/Taskflow` for parallelization
- **Sub-version granularity** - Engine profiles must be specific (e.g., v3.17.2 not just v3)

</details>

## License

This project is licensed under the MIT License.

---
