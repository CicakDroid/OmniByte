https://github.com/CicakDroid/OmniByte/blob/3759e00d119ccb861604479fddf3d8b311c1d793/omnibyte-icon-chip-angkasa.svg

# OmniByte

Android project: static & dynamic analysis (HydraDis) + live-device runtime + game-engine
asset dumper, with a shared C++17 core wired through Gradle CMake.

## Build wiring

- `app/` — Android application module (Kotlin, `com.omnibyte.app`).
  `externalNativeBuild` points to `app/src/main/cpp/CMakeLists.txt`, which
  aggregates the three native roots into one `libomnibyte.so`.
- `engine-core/` — static analysis (`HydraDis/`: Disassembler, Decompiler,
  Parser, Plugin). Backend adapters (capstone, rizin, lief, rz-ghidra, triton,
  z3, cvc5) are placeholders with commented `find_package`/`ExternalProject_Add`
  blocks.
- `runtime/` — live-device runtime (Bridges, ProcessManager, MemoryIO,
  SymbolResolver). Placeholder targets.
- `modules/Dumper/` — dumper module (DumperCore, 7 Engines, Export/Writers).
  Placeholder targets.

C++17; ABIs: `arm64-v8a`, `armeabi-v7a`. STL: `c++_shared`.

Each native root owns its own `CMakeLists.txt` and is a static placeholder
library, linked together in the app aggregator. Enable a real backend by
uncommenting the wiring block inside the corresponding adapter
`CMakeLists.txt`.
