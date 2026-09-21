# Laporan Verifikasi Refactoring Hydra2D

**Tanggal:** 20 September 2026
**Status:** Menunggu Keputusan Atasan

---

## 1. Ringkasan Pekerjaan Yang Sudah Dilakukan

### 1.1 Refactoring Plugin → Analysis (SELESAI ✅)

Seluruh logic deteksi telah dipindahkan dari Plugin ke Analysis classes. Ketiga Plugin sekarang bersih (murni orkestrator + JSON formatter):

| Plugin | Jumlah Baris | Logic Deteksi? | Status |
|--------|-------------|----------------|--------|
| Renamer.cpp | 235 | ❌ Tidak ada | ✅ Selesai |
| RTTI.cpp | 142 | ❌ Tidak ada | ✅ Selesai |
| CFG.cpp | 211 | ❌ Tidak ada | ✅ Selesai |

**Pola yang diterapkan:** buat analyzer → panggil analyzer → format JSON. Tidak ada loop deteksi mentah, tidak ada scanning prologue, tidak ada deteksi PLT/GOT di kode plugin.

### 1.2 Analysis Classes (SELESAI ✅)

Semua Analysis classes sudah memiliki implementasi lengkap:

| Class | Jumlah Baris | Status |
|-------|-------------|--------|
| Functions.cpp | 285 | ✅ Dua overload: `(entryAddr, codeData)` + `(codeBaseAddr, codeData, symbols)` |
| Variables.cpp | 104 | ✅ `analyzeVariables(entryAddress, codeData)` + helper functions |
| Parameters.cpp | 97 | ✅ `analyzeParameters(entryAddress, codeData)` + helper functions |
| Strings.cpp | 112 | ✅ `analyzeStrings(data, dataSize)` + classification helpers |
| Imports.cpp | 220 | ✅ `analyzeImports(codeBaseAddr, codeData, symbols)` + PLT/GOT detection |
| Confidences.cpp | 109 | ✅ `analyzeConfidences(codeData)` + scoring helpers |
| Types.cpp | 436 | ✅ `analyzeTypes()`, `analyzeVtables()`, `recoverTypes()` + full RTTI recovery |
| List.cpp | 483 | ✅ DFS, BFS, SCC, `buildCFGFromDisassembly()`, liveness, reaching defs |
| Tree.cpp | 256 | ✅ Dominator tree, loop detection, dominance frontiers, value numbering |

---

## 2. Perbandingan: Spec vs Implementasi Saat Ini

### 2.1 API Mismatches

| No. | Spec Membutuhkan | Implementasi Saat Ini | Selisih |
|-----|------------------|----------------------|---------|
| 1.2 | `Variables::analyzeStackVariables(codeData, functionsResult)` | `Variables::analyzeVariables(entryAddress, codeData)` | Tidak ada dependency FunctionsResult |
| 1.3 | `Parameters::classifyRegisters(codeData, functionsResult)` | `Parameters::analyzeParameters(entryAddress, codeData)` | Tidak ada dependency FunctionsResult |
| 1.6 | `Confidences::scoreAnalysis(functionsResult, variablesResult, ...)` | `Confidences::analyzeConfidences(codeData)` | Tidak ada dependency hasil analisis lain |
| Bag.2 | `Types::analyzeRTTI(functionsResult)` | `Types::recoverTypes(symbols, sections)` | Parameter input berbeda |
| Bag.3 | `CfgAnalyzer::buildControlFlowGraph(functionsResult)` | Terpecah: `List::buildCFGFromDisassembly()` + `Tree::computeDominatorTree()` | Tidak ada CfgAnalyzer unified |

### 2.2 Pendekatan Desain

| Aspek | Spec | Implementasi Saat Ini |
|-------|------|----------------------|
| **Ketergantungan** | Analysis classes saling bergantung (FunctionsResult → Variables, Parameters) | Setiap Analysis class berdiri sendiri (independen) |
| **Ketahanan** | Perubahan di Functions akan mempengaruhi Variables, Parameters | Perubahan di Functions TIDAK mempengaruhi classes lain |
| **Ketestatikan** | Lebih kompleks (perlu orchestrate dependencies) | Lebih sederhana (setiap class bisa dipanggil sendiri) |

---

## 3. Pertanyaan Kritis Yang Perlu Dijawab

### 3.1 Tentang Duplikasi Fungsi

> **Catatan:** "Jika ada permintaen atau pembuatan Analysis yang berpotensi jadi sia-sia, double fungsi dari analysis yang sudah ada. katakan kepada saya dan jangan dibuatkan."

Pertanyaan: **Apakah perubahan API spec harus diimplementasikan meskipun akan membuat duplikasi fungsi?**

Contoh:
- `analyzeStackVariables(codeData, functionsResult)` pada dasarnya = `analyzeVariables` + awareness FunctionsResult → tetapi `analyzeVariables` sudah bekerja secara independen
- `classifyRegisters(codeData, functionsResult)` pada dasarnya = `analyzeParameters` + awareness FunctionsResult → tetapi `analyzeParameters` sudah bekerja secara independen

### 3.2 Pilihan Pendekatan

| Pilihan | Deskripsi | Konsekuensi |
|---------|-----------|-------------|
| **A** | Pertahankan desain saat ini | Analysis classes independen, tidak ada coupling, tidak ada duplikasi |
| **B** | Tambah overload baru sesuai spec | Method baru yang mengambil `FunctionsResult` sebagai input, wrapper tipis di atas logic yang sudah ada |
| **C** | Ganti method yang ada sesuai spec | Breaking change, lebih invasif, mengubah API yang sudah dipakai |

### 3.3 Pertanyaan Tambahan

1. **Build Verification:** Saat ini build tidak bisa dijalankan karena:
   - Header Boost (`boost/crc.hpp`) tidak tersedia di sandbox
   - PatternScanner memiliki type mismatch (`vector<bool>` vs `vector<uint8_t>`)
   - DumperManager memiliki include path yang rusak

   **Pertanyaan:** Apakah perlu diperbaiki dulu sebelum verifikasi, atau cukup verifikasi logic saja?

2. **Unit Test:** Setiap Analysis class memerlukan minimal 1 unit test independent dari Plugin/PluginContext.

   **Pertanyaan:** Apakah unit test sudah ada di lokasi yang benar, atau perlu dibuat baru?

3. **BoostAdapter:** Keputusan tarball sudah dikonfirmasi. `FETCHCONTENT_SOURCE_DIR_BOOST=<path>` untuk offline sandbox.

   **Pertanyaan:** Apakah BoostAdapter perlu diintegrasikan ke dalam refactoring ini, atau bisa dikerjakan terpisah?

---

## 4. Rekomendasi

### 4.1 Untuk Analysis Module

| Item | Prioritas | Status |
|------|-----------|--------|
| Export semua Result structs dari IAnalysis.h | Tinggi | ✅ Sudah dilakukan |
| Vtables interface di IAnalysis.h | Sedang | ⚠️ Perlu verifikasi |
| Imports interface konsisten | Sedang | ⚠️ Perlu verifikasi |

### 4.2 Untuk Common Library

| Item | Prioritas | Status |
|------|-----------|--------|
| Aho-Corasick adapter (~200 LOC header-only) | Tinggi | Direkomendasikan |
| Graph abstractions | Sedang | Medium priority |
| Compression adapter | Rendah | YAGNI, tunda |

---

## 5. Kesimpulan

**Refactoring utama (Plugins → Analysis) sudah SELESAI.** Plugins sekarang murni orkestrator, semua logic deteksi ada di Analysis classes.

**Yang belum diputuskan:** Apakah API Analysis classes perlu diubah sesuai spec (yang akan membuat duplikasi), atau cukup dipertahankan desain independen saat ini.

**Menunggu keputusan atasan untuk melanjutkan.**

---

*Dokumen ini dibuat untuk verifikasi dan meminta keputusan dari atasan.*
