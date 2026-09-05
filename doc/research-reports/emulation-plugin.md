# Laporan Riset: Enhanced/Emulation Plugin

**Tanggal:** 2026-09-05
**Status:** Implementasi selesai (wrapper + plugin utama)

## 1. Unicorn Engine

### Versi Stabil
- **Versi terbaru:** v2.1.4 (released 2025-09-09)
- **Basis:** QEMU 5 (modified)
- **Lisensi:** GPLv2
- **Platform:** Linux, macOS, Windows, Android (cross-compile via NDK)
- **Arsitektur:** ARM, ARM64, x86, x86_64, MIPS, PPC, SPARC, SystemZ, dan lainnya

### Sumber Resmi
- **Repository:** https://github.com/unicorn-engine/unicorn
- **Dokumentasi:** https://www.unicorn-engine.org/docs/
- **Contoh:** https://github.com/unicorn-engine/unicorn/tree/master/samples

### Cross-compile Android NDK
```bash
# Syarat: Android NDK r25+ terinstall
export NDK=/path/to/android-ndk-r25b

git clone https://github.com/unicorn-engine/unicorn.git
cd unicorn
mkdir build && cd build

cmake -DCMAKE_TOOLCHAIN_FILE=$NDK/build/cmake/android.toolchain.cmake \
      -DANDROID_ABI=arm64-v8a \
      -DANDROID_PLATFORM=android-24 \
      -DCMAKE_BUILD_TYPE=Release \
      -DUNICORN_BUILD_SHARED=OFF \
      -DUNICORN_BUILD_TESTS=OFF \
      ..

cmake --build . -j$(nproc)
```

### API yang Digunakan
| Fungsi | Deskripsi |
|--------|-----------|
| `uc_open()` | Buka instance Unicorn Engine |
| `uc_mem_map()` | Alokasi memory untuk code/stack |
| `uc_mem_write()` | Tulis bytes instruksi ke memory |
| `uc_emu_start()` | Mulai eksekusi instruksi |
| `uc_reg_read/write()` | Baca/tulis register |
| `uc_hook_add()` | Tambah hook (code, memory access) |
| `uc_hook_del()` | Hapus hook |
| `uc_close()` | Tutup instance |

### Status File

| File | Status | Keterangan |
|------|--------|------------|
| `Unicorn/unicorn_engine.h` | Written & functional | Header lengkap dengan tipe data dan API |
| `Unicorn/unicorn_engine.cpp` | Written & functional | Implementasi wrapper dengan stub mode untuk compile tanpa Unicorn |
| `Unicorn/CMakeLists.txt` | Written & functional | Build system dengan auto-detect Unicorn |

## 2. qemu-anti-detection

### Versi Stabil
- **Patch terbaru:** `qemu-10.2.2.patch` untuk QEMU 10.2.x
- **Patch tersedia untuk:** QEMU 6.2.0 - 10.2.2
- **Fungsi:** Menyembunyikan artefak VM dari deteksi anti-cheat

### Sumber Resmi
- **Repository:** https://github.com/zhaodice/qemu-anti-detection
- **Patch:** https://github.com/zhaodice/qemu-anti-detection/tree/master/patches

### Artefak yang Disembunyikan
| Artefak | Metode Deteksi | Metode Penyembunyian |
|---------|----------------|---------------------|
| CPUID Signature | CPUID instruction return "KVMKVMKVM" | Modifikasi CPUID response |
| Hypervisor Bit | CPUID leaf 1, ECX bit 31 | Clear hypervisor present bit |
| BIOS Vendor | SMBIOS/DMI strings | Ganti string "QEMU" dengan vendor real |
| System Vendor | SMBIOS/DMI strings | Ganti string vendor |
| Device ID | /sys/devices/ | Modifikasi device signature |
| Keyboard | /dev/input/ | Rename device name |
| Mouse | /dev/input/ | Rename device name |
| Disk Serial | ATA IDENTIFY | Ganti serial number |
| MAC Address | Network device OUI | Ganti vendor prefix |

### Cross-compile Android NDK
qemu-anti-detection adalah **patch untuk QEMU**, bukan library standalone.
Wrapper kita mengimplementasi anti-detection di level Unicorn (QEMU-based)
dengan cara:
1. Modifikasi register state sebelum eksekusi
2. Hook CPUID instruction untuk return custom values
3. Modifikasi memory yang berisi DMI/SMBIOS strings

### Status File

| File | Status | Keterangan |
|------|--------|------------|
| `Qemu/qemu_antidetect.h` | Written & functional | Header dengan tipe data anti-detection |
| `Qemu/qemu_antidetect.cpp` | Written & functional | Implementasi wrapper dengan validasi artefak |
| `Qemu/CMakeLists.txt` | Written & functional | Build system |

## 3. Plugin Utama (Enhanced/Emulation)

### Alur Kerja
1. Terima `Instruction[]` dari IDisassembler via `PluginContext`
2. Kumpulkan bytes instruksi dari semua sections
3. Deteksi arsitektur dari binary header (ARM64, ARM, x86, x86_64)
4. Baca konfigurasi dari `PluginContext::config`
5. Inisialisasi Unicorn Engine untuk arsitektur target
6. Konfigurasi anti-detection (QemuAntiDetect) jika diminta
7. Map instruksi bytes ke memory Unicorn
8. Eksekusi instruksi dengan timeout dan max instruction limit
9. Kumpulkan trace (register state per-instruksi)
10. Return trace + state akhir sebagai JSON

### Konfigurasi
| Key | Default | Deskripsi |
|-----|---------|-----------|
| `emulation_timeout_us` | `30000000` (30s) | Timeout eksekusi dalam microseconds |
| `emulation_max_instr` | `100000` | Maksimal instruksi yang dieksekusi |
| `emulation_antidetect` | `false` | Aktifkan anti-detection mode |
| `emulation_code_base` | `0x10000` | Base address untuk mapping code |

### Status File

| File | Status | Keterangan |
|------|--------|------------|
| `enhanced_emulation.cpp` | Written & functional | Plugin utama yang wire Unicorn + Qemu |
| `CMakeLists.txt` | Written & functional | Build system dengan subdirectory Unicorn + Qemu |

## 4. Dependensi

### Compile-time (stubs tersedia)
- Unicorn Engine headers (kondisional via `UNICORN_FOUND`)
- Disassembler/IDisassembler.h ( Instruction struct)
- Plugin/IPlugin.h (PluginContext, PluginResult)

### Runtime (wajib install)
- **Unicorn Engine v2.1.4** — tanpa ini, emulation akan gagal (stub mode)
- Build tanpa Unicorn: kompile OK, runtime `result.success = false`

### Build Commands
```bash
# Build full (dengan Unicorn)
cmake -DUNICORN_FOUND=ON -DCMAKE_TOOLCHAIN_FILE=$NDK/... ..
cmake --build .

# Build stub (tanpa Unicorn)
cmake -DCMAKE_TOOLCHAIN_FILE=$NDK/... ..
cmake --build .
# → compile OK, runtime: "Unicorn engine not initialized"
```

## 5. Integrasi dengan SymbolicExecution

Plugin Emulation bisa membantu Triton-based SymbolicExecution dengan:
1. **Concrete trace**: Menjalankan instruksi secara konkret untuk mendapatkan register/memory state
2. **Path exploration**: Menjalankan instruksi di berbagai kondisi awal
3. **Anti-detection**: Menyembunyikan artefak VM saat emulation

Integrasi dilakukan melalui `PluginContext` — kedua plugin menerima data yang sama dari IDisassembler.
