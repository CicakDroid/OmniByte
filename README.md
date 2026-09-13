<p align="center">
  <img src="assets/icon.svg" width="128" alt="OmniByte Icon"/>
</p>

<h1 align="center">OmniByte</h1>

<p align="center">
  <strong>Toolkit Reverse Engineering Android</strong><br/>
  Dekompilator &bull; Editor &bull; Dumper &bull; Hooking &bull; Editor Memori &bull; Monitoring Jaringan
</p>

<p align="center">
  <strong>🇮🇩 Bahasa Indonesia</strong> &bull;
  <a href="README_EN.md">🇬🇧 English</a> &bull;
  <a href="README_PT.md">🇧🇷 Portugu&ecirc;s</a> &bull;
  <a href="README_RU.md">🇷🇺 Русский</a> &bull;
  <a href="README_ZH.md">🇨🇳 中文</a>
</p>

---

## Tentang

OmniByte adalah toolkit reverse engineering Android berbasis **Kotlin + C++ Native** yang mendukung multi-arsitektur (**ARMv7** & **ARMv8a**) dan multi-versi Android (**6.0 hingga versi terakhir**).

Toolkit ini dirancang untuk analisis statis & dinamis, dekompilasi, editasi biner, hooking fungsi, editasi memori, serta monitoring, penangkapan & editasi jaringan — semua dalam satu platform terpadu.

## Fitur Utama

| Fitur | Deskripsi |
|-------|-----------|
| 🔍 **APK Decompiler** | Dekompilasi APK ke source code (Java/Smali/DEX) dengan dukungan multi-engine |
| ✏️ **APK Editor** | Penampil & manipulasi manifest, resource, smali, dan rebuild APK |
| 📊 **Analisis Biner** | Analisis statis & dinamis biner (ELF/PE) dengan disassembler & decompiler |
| 🔧 **Editor Biner** | Penampil, ekplorasi & manipulasi biner langsung dengan hex editor & patching |
| 📦 **Binary Dumper** | Dump struktur biner secara manual atau otomatis saat Live PID |
| 🪝 **Hooking** | Hook fungsi native & ART method dengan 4 teknik berbeda |
| 🧠 **Editor Memori** | Baca & tulis memori proses live |
| 🌐 **Monitoring, Penangkapan & Editasi Jaringan** | Monitor, tangkap, dan edit paket jaringan secara real-time |

## Dukungan Platform

| Komponen | Dukungan |
|----------|----------|
| **Android** | 6.0 (API 23) — versi terakhir |
| **Arsitektur** | ARMv7 (32-bit), ARMv8a (64-bit) |
| **Bahasa** | Kotlin (UI), C++17 (Native Core) |
| **Build System** | Gradle + CMake |
| **STL** | c++_shared |

## Struktur Proyek

```
OmniByte/
├── app/                                 # Aplikasi Android (Kotlin)
│   └── src/main/
│       ├── java/com/omnibyte/           # Source Kotlin
│       ├── cpp/                         # Native aggregator (CMake)
│       └── res/                         # Resource Android
├── Hydra/                               # Mesin Analisis Statis & Dinamis
│   ├── Hydra2D/                         # Mesin analisis inti
│   │   ├── Disassembler/                # Disassembly biner
│   │   ├── Decompiler/                  # Dekompilasi kode
│   │   ├── Parser/                      # Parsing format biner
│   │   ├── Orchestrator/                # Orkestrasi analisis
│   │   ├── Factory/                     # Faktor komponen
│   │   ├── Plugin/                      # Plugin analisis
│   │   │   ├── Enhanced/                # Plugin lanjutan
│   │   │   │   ├── AST/                 # Abstract Syntax Tree
│   │   │   │   ├── CFG/                 # Control Flow Graph
│   │   │   │   ├── Crypt/               # Deteksi kriptografi
│   │   │   │   ├── Deobfuscate/         # Deteksi & penghilang obfuskasi
│   │   │   │   ├── Emulation/           # Emulasi biner
│   │   │   │   ├── FunctionResolver/.   #
│   │   │   │   ├── RTTI/                # Runtime Type Info
│   │   │   │   ├── Signatures/          # Pola tanda tangan
│   │   │   │   └── SymbolicExecution/
│   │   │   └── ScriptHooks/             # Hook berbasis skrip
│   │   └── docs/
│   └── Shared/                           # Metadata bersama
├── modules/
│   ├── Dumper/                           # Modul Binary Dumper
│   │   ├── DumperCore/                   # Logika inti dumper
│   │   │   ├── Detector/                 # Deteksi engine
│   │   │   ├── EngineRegistry/           # Registrasi engine
│   │   │   ├── ResultNormalizer/         # Normalisasi output
│   │   │   ├── SignatureBypass/          # Bypass tanda tangan APK
│   │   │   ├── SharedUtils/              # Utilitas
│   │   │   └── WorkingModes/             # Mode Manual & Live
│   │   ├── Engines/                      # 7 Engine Dumper
│   │   │   ├── UnityIL2CPP/              # Unity IL2CPP
│   │   │   ├── UnityMono/                # Unity Mono
│   │   │   ├── UnrealEngine/             # Unreal Engine
│   │   │   ├── Godot/                    # Godot Engine
│   │   │   ├── Cocos2d/          # Cocos2d
│   │   │   ├── GameMaker/        # GameMaker
│   │   │   └── Source2/          # Source 2
│   │   └── Export/               # Penulis hasil
│   ├── Hooker/                   # Modul Hooking
│   │   ├── HookEngine/           # Teknik hook
│   │   │   ├── ARTHook/          # Hook metode ART
│   │   │   ├── InlineHook/       # Inline hook fungsi
│   │   │   ├── PLT_GOTHook/      # PLT/GOT hook
│   │   │   └── TracelessHook/    # Hook anti-deteksi
│   │   └── backends/             # Backend hook
│   │       ├── Albatross/        # Backend ART hook
│   │       ├── Bhook/            # Backend PLT/GOT
│   │       ├── Inlinehook/       # Backend inline hook
│   │       ├── KittyMemory/      # Manipulasi memori
│   │       ├── KittyMemoryEx/    # Ekstensi manipulasi memori
│   │       └── Vector/           # Backend traceless
│   └── HPT/                      # Modul HPT
│       ├── Hooker/               # Wrapper hook
│       └── MemoryEditor/         # Editor memori
├── runtime/                      # Runtime Perangkat Live
│   ├── Bridges/                  # Pemuat jembatan
│   │   ├── FreedomServiceBridge/ # Jembatan layanan root
│   │   └── ModuleBridge/         # Jembatan modul
│   ├── ProcessManager/           # Manajemen proses
│   ├── MemoryIO/                 # Baca/tulis memori
│   ├── SymbolResolver/           # Resolusi simbol (xdl)
│   ├── ZigZag/                   # Mesin stealth/bypass
│   ├── ZigZagManager/            # Manajemen stealth
│   └── FreedomService/           # Layanan akses root
│       ├── KernelSU-Next/        # Dukungan KernelSU
│       ├── SUI/                  # Dukungan Magisk SUI
│       ├── SukiSU-Ultra/         # Dukungan SukiSU
│       └── RootThread/           # Manajemen thread root
├── common/                       # Utilitas bersama
│   ├── Math/                     # Utilitas matematika
│   └── Json/                     # Serialisasi
├── toolchain/                    # Toolchain build
│   ├── rizin-android/            # Disassembler Rizin
│   └── stub-headers/             # Header stub
├── scripts/                      # Skrip build
├── docs/                         # Dokumentasi
└── test/                         # Bahan untuk test
```

## Pipeline Kerja

```mermaid
flowchart TB
    subgraph INPUT["Input"]
        APK["File APK"]
        BIN["Biner (ELF/PE)"]
        PID["Proses Live (PID)"]
    end

    subgraph HYDRA["Hydra - Analisis Statis"]
        direction TB
        H_DIS["Disassembler\n(Capstone / Rizin)"]
        H_DEC["Decompiler\n(Rizin / rz-ghidra)"]
        H_PAR["Parser\n(LIEF)"]
        H_PLG["Plugins\n(AST, CFG, Crypt,\nDeobfuscate, Signatures,\nSymbolicExecution)"]
        H_ORC["Orkestrator"]
        H_DIS --> H_ORC
        H_DEC --> H_ORC
        H_PAR --> H_ORC
        H_PLG --> H_ORC
    end

    subgraph DUMPER["Dumper - Dumper Biner"]
        direction TB
        D_DET["Detektor\n(Magic Bytes,\nVerifikasi Versi)"]
        D_ENG["Engines\n(UnityIL2CPP, UnityMono,\nUnrealEngine, Godot,\nCocos2d, GameMaker, Source2)"]
        D_RES["Resolver\n(Registrasi, Simbol)"]
        D_EXP["Export\n(CSharpWriter, StructGen)"]
        D_DET --> D_ENG
        D_ENG --> D_RES
        D_RES --> D_EXP
    end

    subgraph HOOKER["Hooker - Hooking Fungsi"]
        direction TB
        HK_ART["ART Hook\n(Albatross)"]
        HK_INL["Inline Hook\n(android-inline-hook)"]
        HK_PLT["PLT/GOT Hook\n(Bhook)"]
        HK_TLS["Traceless Hook\n(Vector)"]
        HK_ART --> HK_MEM["Patching Memori\n(KittyMemory)"]
        HK_INL --> HK_MEM
        HK_PLT --> HK_MEM
        HK_TLS --> HK_MEM
    end

    subgraph RUNTIME["Runtime - Perangkat Live"]
        direction TB
        R_BRG["Jembatan\n(FreedomService, Module)"]
        R_PM["ProcessManager"]
        R_MIO["MemoryIO"]
        R_SYM["SymbolResolver\n(xdl)"]
        R_ZZ["ZigZag\n(Stealth/Bypass)"]
        R_FS["FreedomService\n(KernelSU, SUI, SukiSU)"]
        R_BRG --> R_PM
        R_PM --> R_MIO
        R_PM --> R_SYM
        R_ZZ --> R_FS
    end

    subgraph OUTPUT["Output"]
        O_CS["dump.cs\n(Definisi C#)"]
        O_ST["struct_dump.cs\n(Layout Struct)"]
        O_SF["static_fields.txt\n(Offset Statis)"]
        O_SM["Source Smali"]
        O_PT["Hasil Patch"]
    end

    APK --> H_PAR
    BIN --> H_DIS
    BIN --> D_DET
    PID --> R_PM

    H_ORC --> D_DET
    D_EXP --> O_CS
    D_EXP --> O_ST
    D_EXP --> O_SF
    H_DEC --> O_SM
    HK_MEM --> O_PT
    R_MIO --> HK_MEM
    R_SYM --> D_RES
```

## Arsitektur Modul

```mermaid
flowchart LR
    subgraph CORE["Lapisan Core"]
        COMMON["common/\n(Math, Serialization)"]
        TOOLCHAIN["toolchain/\n(Rizin, Headers)"]
    end

    subgraph NATIVE["Lapisan Native (C++17)"]
        HYDRA["Hydra / Hydra2D\nAnalisis Statis"]
        DUMPER["Dumper\nDumping Biner"]
        HOOKER["Hooker\nHooking Fungsi"]
        RUNTIME["Runtime\nPerangkat Live"]
    end

    subgraph UI["Lapisan UI (Kotlin)"]
        APP["app/\nAplikasi Android"]
    end

    CORE --> HYDRA
    CORE --> DUMPER
    CORE --> HOOKER
    CORE --> RUNTIME
    TOOLCHAIN --> HYDRA

    HYDRA --> DUMPER
    RUNTIME --> DUMPER
    RUNTIME --> HOOKER

    HYDRA -.-> APP
    DUMPER -.-> APP
    HOOKER -.-> APP
    RUNTIME -.-> APP
```

## Build

```bash
# Clone
git clone https://github.com/CicakDroid/OmniByte.git
cd OmniByte

# Build APK
./gradlew assembleDebug

# Atau build native only
cd app/src/main/cpp
mkdir build && cd build
cmake -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
      -DANDROID_ABI=arm64-v8a \
      -DANDROID_PLATFORM=android-23 \
      ..
make
```

## Lisensi

Proyek ini merupakan bagian dari OmniByte Research Platform.

---

<p align="center">
  <sub>Dibuat dengan ❤️ oleh <a href="https://github.com/CicakDroid">CicakDroid</a></sub>
</p>
