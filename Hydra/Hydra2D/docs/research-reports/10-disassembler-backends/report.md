# Laporan Riset 10 — Disassembler Backend Adapters (TAHAP 21 & 22)

- **Tanggal**: 2026-09-28
- **Cakupan**: `CapstoneAdapter`, `ProtobufAdapter`, `WasmAdapter` + integrasi factory dispatch (`HydraDisassembler.cpp`), enum `DisassemblerArch`
- **Status**: Selesai — semua gate GREEN
- **Toolchain**: g++ 15.2.0, CMake 4.4.3, capstone (FetchContent), protobuf v25.3, wabt (commit `02418f72`)

---

## 1. Hasil Gate

| Gate | Cakupan | Hasil |
|---|---|---|
| **A** — syntax-only factory dispatch | `HydraDisassembler.cpp` | **EXIT=0** |
| **C** — full sweep | `Disassembler/*.cpp` + 3 adapter + 3 file `Factory/*.cpp` | **EXIT=0** (sisa warning pre-existing `is64Bit` di `DisassemblerFactory.cpp:14`) |
| **B** — CMake real build | configure + target `hydradis_disassembler` + `hydradis_factory` | **configure EXIT=0, build EXIT=0, 100%** |

Artefak terverifikasi di `/tmp/opencode/gateb/build/`:

- `libhydradis_disassembler.a` (147 KB) — `HydraDisassembler.cpp` kompilasi terhadap ketiga adapter
- `libhydradis_factory.a` (544 KB) — build `hydradis_factory` menarik dependency `hydradis_disassembler`, bukti link `Factory/CMakeLists.txt:25` ke target nyata (bukan `-l` menggantung)
- `libdisassembler_capstone.a` (576 KB), `libdisassembler_protobuf.a` (1.26 MB), `libdisassembler_wasm.a` (1.88 MB) — semua dari source asli (capstone, protobuf + abseil inline, wabt), FetchContent offline via `FETCHCONTENT_SOURCE_DIR_*`

Catatan gate:

- Gate C dijalankan tanpa `Factory/DecompilerFactory.cpp` karena rusak oleh rename pre-existing (lihat §4).
- Gate B memakai harness superbuild `/tmp/opencode/gateb` (2 file di `/tmp`, nol edit repo) + forwarding shim 2 baris untuk `Decompiler/IDecompiler.h`, karena root configure FATAL oleh masalah yang sama.
- Mesin 8 core / 3.7 Gi RAM: build `-j8` memicu GCC ICE (segfault di `wabt/src/opcode.cc`), `-j2` aman.

## 2. Pekerjaan Selesai

- **TAHAP 22**: `DisassemblerArch` `None`/`WASM` ditambahkan (enum kini 16 case), `mapArch()` lengkap, `-Werror=switch` pass.
- **Bagian 1 — CapstoneAdapter** ✅: adapter + `mapArch`, build real pass.
- **Bagian 2 — ProtobufAdapter** ✅: stub gate + build real (perbaikan real-API `ByteCount`).
- **Bagian 3 — WasmAdapter** ✅: streaming delegate `BinaryReaderNop`, stub wabt, perbaikan `::v128`, build real.
- **Bagian 4 — Integrasi** ✅:
  - `HydraDisassembler.cpp`: 2 include + case Protobuf/WASM ter-wire ke `createProtobufDisassembler`/`createWasmDisassembler` (TODO `nullptr` hilang)
  - `Disassembler/CMakeLists.txt`: target STATIC baru `hydradis_disassembler`
  - `Factory/CMakeLists.txt:25` → `hydradis_disassembler`
- **Kontrak tidak diubah**: signature `createDisassembler`/`IDisassembler` identik, sesuai aturan spec (perubahan kontrak wajib dilaporkan dulu).

## 3. Bersih-bersih temp build (atas permintaan)

Dihapus: `wabtbuild` (12 M), output build `hbuild` 398→321 M, output build `pbcfg` 233→194 M, log gate lama, `v128test.cc` — **~105 M+ dibebaskan**.

Dipertahankan karena masih dipakai gateb: `hbuild/_deps/{capstone,lief,z3}-src`, `pbcfg/_deps/protobuf-src` (direferensikan cache gateb), `wabt` (source).

Bukan milik TAHAP ini, belum dihapus (menunggu instruksi): `rizin` (89 M) + log rizin/android, `eigen-verify` (38 M) + tarball, `test_analysis`, `baseline_test`, `graph_selfcheck`, `build.log`/`cfg*.log`, `enc*`/`probe.bin`.

## 4. Masalah pre-existing (dilaporkan, TIDAK diperbaiki)

1. **Rename Decompiler in-flight merusak root build**:
   - `Factory/DecompilerFactory.h:6` masih `#include "Decompiler/IDecompiler.h"` (file dihapus → penerus `HydraDecompiler.h`)
   - `Decompiler/backends/CMakeLists.txt` masih `add_subdirectory(rizin-native)` (dir dihapus → `RizinAndroidAdapter/`) → root configure FATAL
   - Dibypass di harness gateb via forwarding shim; fix = 1 kata per file bila diizinkan.
2. **Link menggantung `Factory/CMakeLists.txt:26-27`**: `hydradis_parser_backends`/`hydradis_decompiler_backends` adalah nama `project()`, bukan target (latent: repo tak punya `add_executable`).
3. Warning `unused parameter 'is64Bit'` (`DisassemblerFactory.cpp:14`), breakage lama `unicorn_engine.cpp`/`qemu_antidetect`.

## 5. Menunggu persetujuan

- **Format detection (magic bytes `00 61 73 6D` → WASM)**: tidak ada pemanggil di codebase (Orchestrator memakai backend dari config `disasmBackend`). Menambah fungsi publik baru = perubahan kontrak → **belum diimplementasi**, butuh restu.

## 6. Rekomendasi

- **RAM**: `-j2` untuk build root di mesin ini (3.7 Gi / 8 core; `-j8` memicu GCC ICE).
- **wabt FetchContent**: tambah `GIT_SUBMODULES third_party/picosha2` (dipakai `USE_INTERNAL_SHA256`); submodule init manual diperlukan saat clone pertama.
- **Item wabt yang diskip** (Bagian 3): name-section suffix, init-expr, locals-decl pseudo-entry, `OnTryTableExpr`/`OnDelegateExpr` text, guard LEB128 non-kanonik, abort `count`.
- Harness gateb + log bukti di `/tmp/opencode/gateb/` — boleh dihapus setelah laporan ini dibaca.
