#pragma once
// ── unicorn_engine.h ───────────────────────────────────────────────
// Wrapper untuk Unicorn Engine (CPU emulation).
// Menerima Instruction[] dari IDisassembler dan mengeksekusinya
// dalam environment emulasi yang terisolasi.
//
// Unicorn Engine: https://github.com/unicorn-engine/unicorn
// Versi stabil: 2.1.4 (2025-09-09, QEMU 5 based)
// Lisensi: GPLv2
//
// Cross-compile Android NDK:
//   mkdir build && cd build
//   cmake -DCMAKE_TOOLCHAIN_FILE=$NDK/build/cmake/android.toolchain.cmake \
//         -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-24 \
//         -DCMAKE_BUILD_TYPE=Release ..
//   cmake --build . -j$(nproc)
//
// Source:
//   - unicorn-engine/unicorn: include/unicorn/unicorn.h
//   - unicorn-engine/unicorn: include/unicorn/arm64.h
//   - unicorn-engine/unicorn: samples/sample_arm64.c

#include <cstdint>
#include <string>
#include <vector>
#include <functional>
#include <memory>

// Forward declare Unicorn types (avoid header dependency at compile time)
// Actual include di .cpp ketika Unicorn tersedia
typedef void* uc_engine;
typedef unsigned int uc_err;
typedef unsigned int uc_arch;
typedef unsigned int uc_mode;

namespace omnibyte::hydradis {

// ── Forward declarations ────────────────────────────────────────
struct Instruction;  // from IDisassembler.h

// ── Emulation Trace Types ───────────────────────────────────────

/// Satu entry dalam trace eksekusi.
/// Mencatat state register pada satu titik eksekusi.
struct TraceEntry {
    uint64_t address;               // PC saat entry ini
    std::string instructionText;    // teks instruksi (mnemonic + operands)
    std::vector<uint8_t> bytes;     // raw bytes instruksi

    /// Register state pada titik ini (key = nama register, value = nilai)
    std::unordered_map<std::string, uint64_t> registers;

    /// Memory access yang terjadi (opsional, hanya diisi kalau hook aktif)
    struct MemoryAccess {
        uint64_t address;
        uint32_t size;
        bool isWrite;
        std::vector<uint8_t> value;
    };
    std::vector<MemoryAccess> memoryAccesses;
};

/// Hasil eksekusi emulation.
struct EmulationResult {
    bool success = false;
    std::string errorMessage;

    /// Trace lengkap dari semua instruksi yang dieksekusi
    std::vector<TraceEntry> trace;

    /// State register akhir setelah eksekusi selesai
    std::unordered_map<std::string, uint64_t> finalRegisters;

    /// Total instruksi yang dieksekusi
    size_t instructionsExecuted = 0;

    /// Total waktu eksekusi (microseconds)
    uint64_t executionTimeUs = 0;

    /// Apakah eksekusi dihentikan karena timeout/hook
    bool terminated = false;
    std::string terminationReason;
};

// ── Configuration ───────────────────────────────────────────────

/// Konfigurasi untuk UnicornEngine.
struct UnicornConfig {
    /// Base address untuk mapping code dalam emulasi
    uint64_t codeBaseAddress = 0x10000;

    /// Ukuran memory yang dialokasikan untuk code (bytes)
    size_t codeMemorySize = 0x10000;  // 64KB

    /// Ukuran stack yang dialokasikan (bytes)
    size_t stackSize = 0x10000;  // 64KB

    /// Timeout eksekusi dalam microseconds (0 = unlimited)
    uint64_t timeoutUs = 30000000;  // 30 detik

    /// Maksimal jumlah instruksi yang dieksekusi (0 = unlimited)
    size_t maxInstructions = 100000;

    /// Apakah hook memory access (read/write) untuk trace
    bool hookMemoryAccess = true;

    /// Apakah hook invalid memory access
    bool hookInvalidAccess = true;
};

// ── Unicorn Engine Wrapper ──────────────────────────────────────

/// Wrapper untuk Unicorn Engine yang menerima Instruction[] dari IDisassembler.
///
/// Usage:
///   UnicornEngine engine;
///   if (!engine.initialize(DisassemblerArch::ARM64)) { /* handle error */ }
///   auto result = engine.execute(instructions);
///   for (const auto& entry : result.trace) { /* process trace */ }
///
/// Source: unicorn-engine/unicorn (v2.1.4)
class UnicornEngine {
public:
    UnicornEngine();
    ~UnicornEngine();

    // Non-copyable, movable
    UnicornEngine(const UnicornEngine&) = delete;
    UnicornEngine& operator=(const UnicornEngine&) = delete;
    UnicornEngine(UnicornEngine&& other) noexcept;
    UnicornEngine& operator=(UnicornEngine&& other) noexcept;

    /// Initialize engine untuk arsitektur tertentu.
    /// @param arch  DisassemblerArch dari IDisassembler
    /// @return true jika berhasil
    bool initialize(int arch);  // pakai int untuk avoid depend ke IDisassembler.h di header

    /// Apakah engine sudah ter-initialize
    bool isInitialized() const;

    /// Cleanup semua resource
    void shutdown();

    /// Eksekusi instruksi dari IDisassembler.
    /// @param instructions  vector Instruction dari DisassemblyResult
    /// @param config        konfigurasi emulation (optional, pakai default kalau kosong)
    /// @return EmulationResult dengan trace dan state
    EmulationResult execute(
        const std::vector<Instruction>& instructions,
        const UnicornConfig& config = {}
    );

    /// Eksekusi dari raw bytes (untuk kasus yang butuh custom setup)
    /// @param code      raw instruction bytes
    /// @param baseAddr  alamat virtual awal
    /// @param config    konfigurasi emulation
    EmulationResult executeRaw(
        const std::vector<uint8_t>& code,
        uint64_t baseAddr,
        const UnicornConfig& config = {}
    );

    /// Baca register tertentu setelah eksekusi
    /// @param regName   nama register (mis. "x0", "pc", "sp")
    /// @param value     output value
    /// @return true jika berhasil
    bool readRegister(const std::string& regName, uint64_t& value) const;

    /// Tulis register tertentu
    bool writeRegister(const std::string& regName, uint64_t value);

    /// Baca memory
    bool readMemory(uint64_t address, size_t size, std::vector<uint8_t>& out) const;

    /// Tulis memory
    bool writeMemory(uint64_t address, const std::vector<uint8_t>& data);

    /// Ambil error message dari kode error Unicorn
    static std::string errorMessage(int errorCode);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace omnibyte::hydradis
