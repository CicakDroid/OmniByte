<p align="center">
  <img src="assets/icon.svg" width="128" alt="OmniByte Icon"/>
</p>

<h1 align="center">OmniByte</h1>

<p align="center">
  <strong>Toolkit de Engenharia Reversa Android</strong><br/>
  Descompilador &bull; Editor &bull; Dumper &bull; Hooking &bull; Editor de Memória &bull; Monitoramento de Rede
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

OmniByte é um toolkit de engenharia reversa Android construído com **Kotlin + C++ Nativo**, suportando multi-arquitetura (**ARMv7** & **ARMv8a**) e multi-versão Android (**6.0 até a versão mais recente**).

O toolkit é projetado para análise estática e dinâmica, descompilação, edição de binários, hooking de funções, edição de memória e monitoramento, captura e edição de rede — tudo em uma única plataforma integrada. **Sem lifting para LLVM**.

**Funcionalidades Especiais:**
- ✅ **Executa com root E sem root** — Suporta acesso root (KernelSU, Magisk, SukiSU) e modo sem root via `/proc/pid/mem`
- 🔍 **Universal Dumper para Biblioteca Nativa Android** — Despeja todas as bibliotecas `.so` dos processos Android (libil2cpp.so, libtamarin.so, libunity.so, etc.)
- ⚡ **HPT Orchestrator** — Orquestra hooking & edição de memória via subsystems Hooking/MemoryEditing
- 🔄 **Taskflow Adapter** — Adapta taskflow para agendamento & paralelização

## Principais Funcionalidades

| Funcionalidade | Descrição |
|----------------|-----------|
| 🔍 **APK Decompiler** | Descompilar APK em código-fonte (Java/Smali/DEX) com suporte multi-engine |
| ✏️ **APK Editor** | Editar manifest, recursos, smali e reconstruir APK |
| 📊 **Análise de Binário** | Análise estática de binários (ELF/PE) com descompilador e disassembler |
| 🔧 **Editor de Binário** | Edição direta de binários com hex editor e patching |
| 📦 **Universal Dumper** | Despejar biblioteca nativa Android universal (todos os engines: Unity, Unreal, Godot, etc.) manualmente ou automaticamente durante Live PID |
| 🪝 **Hooking** | Hook de funções nativas e métodos ART com 4 backends (Albatross, Bhook, Vector, KittyMemory) |
| 🧠 **Editor de Memória** | Ler e escrever na memória de processos ativos via KittyMemory/KittyMemoryEx |
| 🌐 **Monitoramento, Captura & Edição de Rede** | Monitore, capture e edite pacotes de rede em tempo real |

## Suporte à Plataforma

| Componente | Suporte |
|------------|---------|
| **Android** | 6.0 (API 23) — Versão mais recente |
| **Arquitetura** | ARMv7 (32 bits), ARMv8a (64 bits) |
| **Linguagens** | Kotlin (UI), C++17 (Núcleo Nativo) |
| **Sistema de Build** | Gradle + CMake |
| **STL** | c++_shared |

## Estrutura do Projeto

```
OmniByte/
├── app/                          # Aplicação Android (Kotlin)
│   └── src/main/
│       ├── java/com/omnibyte/    # Código Kotlin
│       ├── cpp/                  # Agregador nativo (CMake)
│       └── res/                  # Recursos Android
├── Hydra/                        # Motor de Análise Estática
│   ├── Hydra2D/                  # Motor principal de análise
│   │   ├── Disassembler/         # Desmontagem de binários
│   │   ├── Decompiler/           # Descompilação de código
│   │   ├── Parser/               # Parsing de formatos de binário
│   │   ├── Orchestrator/         # Orquestração de análise
│   │   ├── Factory/              # Fábrica de componentes
│   │   ├── Plugin/               # Plugins de análise
│   │   │   ├── Enhanced/         # Plugins avançados
│   │   │   │   ├── AST/          # Árvore de Sintaxe Abstrata
│   │   │   │   ├── CFG/          # Grafo de Fluxo de Controle
│   │   │   │   ├── Crypt/        # Detecção de criptografia
│   │   │   │   ├── Deobfuscate/  # Desobfuscação
│   │   │   │   ├── Emulation/    # Emulação de binários
│   │   │   │   ├── FunctionResolver/
│   │   │   │   ├── RTTI/         # Informação de Tipo em Tempo de Execução
│   │   │   │   ├── Signatures/   # Assinaturas de padrão
│   │   │   │   └── SymbolicExecution/
│   │   │   └── ScriptHooks/      # Hooks baseados em scripts
│   │   └── docs/
│   └── Shared/                   # Metadados compartilhados
├── modules/
│   ├── Dumper/                   # Módulo Universal Dumper
│   │   ├── DumperCore/           # Lógica principal do dumper
│   │   │   ├── Detector/         # Detecção de engine
│   │   │   ├── EngineRegistry/   # Registro de engines
│   │   │   ├── ResultNormalizer/ # Normalização de saída
│   │   │   ├── SignatureBypass/  # Bypass de assinatura APK
│   │   │   ├── SharedUtils/      # Utilitários
│   │   │   └── WorkingModes/     # Modos Manual & Live
│   │   ├── Engines/              # 7 Engines de Dumper
│   │   │   ├── UnityIL2CPP/      # Unity IL2CPP
│   │   │   ├── UnityMono/        # Unity Mono
│   │   │   ├── UnrealEngine/     # Unreal Engine
│   │   │   ├── Godot/            # Godot Engine
│   │   │   ├── Cocos2d/          # Cocos2d
│   │   │   ├── GameMaker/        # GameMaker
│   │   │   └── Source2/          # Source 2
│   │   └── Export/               # Escritores de saída
│   ├── Hooker/                   # Módulo de Hooking
│   │   ├── HookEngine/           # Técnicas de hook
│   │   │   ├── ARTHook/          # Hook de métodos ART
│   │   │   ├── InlineHook/       # Hook inline de funções
│   │   │   ├── PLT_GOTHook/      # Hook PLT/GOT
│   │   │   └── TracelessHook/    # Hook anti-detecção
│   │   └── backends/             # Backends de hook
│   │       ├── Albatross/        # Backend ART hook
│   │       ├── Bhook/            # Backend PLT/GOT
│   │       ├── Inlinehook/       # Backend inline hook
│   │       ├── KittyMemory/      # Patching de memória
│   │       ├── KittyMemoryEx/    # Memória estendida
│   │       └── Vector/           # Backend traceless
│   └── HPT/                      # Módulo HPT Orchestrator
│       ├── Hooker/               # Subsystem de hooking (IHookBackend)
│       └── MemoryEditor/         # Subsystem de edição de memória (IMemoryEditor)
├── runtime/                      # Runtime de Dispositivo Ativo
│   ├── Bridges/                  # Carregadores de ponte
│   │   ├── FreedomServiceBridge/ # Ponte de serviço root
│   │   └── ModuleBridge/         # Ponte de módulo
│   ├── ProcessManager/           # Gerenciamento de processos
│   ├── MemoryIO/                 # Leitura/escrita de memória
│   ├── SymbolResolver/           # Resolução de símbolos (xdl)
│   ├── ZigZag/                   # Motor stealth/bypass
│   ├── ZigZagManager/            # Gerenciamento stealth
│   ├── HPTManager/               # Gerenciamento lifecycle do HPT
│   └── FreedomService/           # Serviço de acesso root
│       ├── KernelSU-Next/        # Suporte KernelSU
│       ├── SUI/                  # Suporte Magisk SUI
│       ├── SukiSU-Ultra/         # Suporte SukiSU
│       └── RootThread/           # Gerenciamento de thread root
├── common/                       # Utilitários compartilhados
│   ├── Math/                     # Utilitários matemáticos
│   ├── Json/                     # Serialização
│   └── Taskflow/                 # Adaptador Taskflow (FetchContent v3.8.0)
├── toolchain/                    # Toolchain de build
│   ├── rizin-android/            # Desassembler Rizin
│   └── stub-headers/             # Headers stub
├── scripts/                      # Scripts de build
├── docs/                         # Documentação
└── test/                         # Fixtures de teste
```

## Pipeline de Trabalho

```mermaid
flowchart TB
    subgraph INPUT["Entrada"]
        APK["Arquivo APK"]
        BIN["Binário (ELF/PE)"]
        PID["Processo Ativo (PID)"]
    end

    subgraph HYDRA["Hydra - Análise Estática"]
        direction TB
        H_DIS["Disassembler\n(Capstone / Rizin)"]
        H_DEC["Decompiler\n(Rizin / rz-ghidra)"]
        H_PAR["Parser\n(LIEF)"]
        H_PLG["Plugins\n(AST, CFG, Crypt,\nDeobfuscate, Signatures,\nSymbolicExecution)"]
        H_ORC["Orquestrador"]
        H_DIS --> H_ORC
        H_DEC --> H_ORC
        H_PAR --> H_ORC
        H_PLG --> H_ORC
    end

    subgraph DUMPER["Universal Dumper - Biblioteca Nativa Android"]
        direction TB
        D_DET["Detector\n(Magic Bytes,\nVerificação de Versão)"]
        D_ENG["Engines\n(UnityIL2CPP, UnityMono,\nUnrealEngine, Godot,\nCocos2d, GameMaker, Source2)"]
        D_RES["Resolver\n(Registro, Símbolo)"]
        D_EXP["Export\n(CSharpWriter, StructGen)"]
        D_DET --> D_ENG
        D_ENG --> D_RES
        D_RES --> D_EXP
    end

    subgraph HPT["HPT Orchestrator"]
        direction TB
        HK_MOD["Subsystem de Hooking\n(IHookBackend)"]
        ME_MOD["Subsystem de Edição de Memória\n(IMemoryEditor)"]
        HK_MOD --> HK_ART["ART Hook\n(Albatross)"]
        HK_MOD --> HK_INL["Inline Hook\n(android-inline-hook)"]
        HK_MOD --> HK_PLT["PLT/GOT Hook\n(Bhook)"]
        HK_MOD --> HK_TLS["Traceless Hook\n(Vector)"]
        HK_ART --> HK_MEM["Patching de Memória\n(KittyMemory)"]
        HK_INL --> HK_MEM
        HK_PLT --> HK_MEM
        HK_TLS --> HK_MEM
        ME_MOD --> ME_KIT["KittyMemory"]
        ME_MOD --> ME_KITX["KittyMemoryEx"]
    end

    subgraph RUNTIME["Runtime - Dispositivo Ativo"]
        direction TB
        R_BRG["Pontes\n(FreedomService, Module)"]
        R_PM["ProcessManager"]
        R_MIO["MemoryIO"]
        R_SYM["SymbolResolver\n(xdl)"]
        R_ZZ["ZigZag\n(Stealth/Bypass)"]
        R_HPT["HPTManager\n(Lifecycle)"]
        R_FS["FreedomService\n(KernelSU, SUI, SukiSU)"]
        R_BRG --> R_PM
        R_PM --> R_MIO
        R_PM --> R_SYM
        R_ZZ --> R_FS
        R_HPT --> HK_MOD
        R_HPT --> ME_MOD
    end

    subgraph TASKFLOW["Taskflow - Agendamento Paralelo"]
        direction TB
        TF["TaskflowAdapter\n(FetchContent v3.8.0)"]
    end

    subgraph OUTPUT["Saída"]
        O_CS["dump.cs\n(Definições C#)"]
        O_ST["struct_dump.cs\n(Layout de Struct)"]
        O_SF["static_fields.txt\n(Offsets Estáticos)"]
        O_SM["Código Smali"]
        O_PT["Resultado do Patch"]
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
    TF --> HK_MOD
    TF --> ME_MOD
```

## Arquitetura de Módulos

```mermaid
flowchart LR
    subgraph CORE["Camada Core"]
        COMMON["common/\n(Math, Json, Taskflow)"]
        TOOLCHAIN["toolchain/\n(Rizin, Headers)"]
    end

    subgraph NATIVE["Camada Nativa (C++17)"]
        HYDRA["Hydra / Hydra2D\nAnálise Estática"]
        DUMPER["Universal Dumper\nDespejo Nativo Android"]
        HPT["HPT Orchestrator\nHooking + MemoryEditing"]
        RUNTIME["Runtime\nDispositivo Ativo"]
    end

    subgraph HOOKS["Backends de Hooking"]
        HK_ART["Albatross\n(Hook ART)"]
        HK_BHK["Bhook\n(PLT/GOT)"]
        HK_VEC["Vector\n(Traceless)"]
        HK_KIT["KittyMemory\n(Patching de Memória)"]
    end

    subgraph UI["Camada UI (Kotlin)"]
        APP["app/\nAplicação Android"]
    end

    CORE --> HYDRA
    CORE --> DUMPER
    CORE --> HPT
    CORE --> RUNTIME
    TOOLCHAIN --> HYDRA

    HYDRA --> DUMPER
    RUNTIME --> DUMPER
    RUNTIME --> HPT
    HPT --> HK_ART
    HPT --> HK_BHK
    HPT --> HK_VEC
    HPT --> HK_KIT

    HYDRA -.-> APP
    DUMPER -.-> APP
    HPT -.-> APP
    RUNTIME -.-> APP
```

## Build

```bash
# Clonar
git clone https://github.com/CicakDroid/OmniByte.git
cd OmniByte

# Build do APK
./gradlew assembleDebug

# Ou build apenas nativo
./scripts/build-native.sh

# Build com opções específicas
./scripts/build-native.sh --abi arm64-v8a --api 23
./scripts/build-native.sh --abi armeabi-v7a --api 21
./scripts/build-native.sh --clean
```

> **Nota:** Build nativo requer Android SDK & NDK. Execute `./scripts/build-native.sh --help` para todas as opções.

## Licença

Este projeto é parte da OmniByte Research Platform.

---

<p align="center">
  <sub>Feito com ❤️ por <a href="https://github.com/CicakDroid">CicakDroid</a></sub>
</p>
