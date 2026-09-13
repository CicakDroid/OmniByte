#pragma once
// ── qemu_antidetect.h ────────────────────────────────────────────
// Wrapper untuk qemu-anti-detection mode dalam emulation.
// Mengkonfigurasi environment emulasi untuk menyembunyikan artefak
// yang terdeteksi oleh anti-cheat/anti-VM.
//
// qemu-anti-detection: https://github.com/zhaodice/qemu-anti-detection
// Patch tersedia untuk QEMU 6.2.0 - 10.2.2
// Fungsi: menyembunyikan artefak VM (CPUID, keyboard, mouse, dll)
//
// Cross-compile Android NDK:
//   qemu-anti-detection adalah patch untuk QEMU, bukan library standalone.
//   Untuk wrapper ini, kita implement anti-detection logic di level Unicorn:
//   - Modifikasi register state (CPUID equivalent)
//   - Sembunyikan artefak di memory layout
//   - Konfigurasi environment agar tidak terdeteksi
//
// Source:
//   - zhaodice/qemu-anti-detection: README.md
//   - zhaodice/qemu-anti-detection: patches/

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

namespace omnibyte::hydradis {

// ── Anti-Detection Types ───────────────────────────────────────

/// Tipe artefak VM yang bisa dideteksi
enum class VMArtifactType {
    CPUIDSignature,     // CPUID menunjukkan "KVMKVMKVM" atau "Microsoft Hv"
    BIOSVendor,         // BIOS vendor string
    SystemVendor,       // System vendor string
    HypervisorBit,      // CPUID hypervisor present bit
    DeviceSignature,    // Device ID menunjukkan virtio/QEMU devices
    KeyboardIndicator,  // Keyboard device name/ID
    MouseIndicator,     // Mouse device name/ID
    DMIString,          // DMI/SMBIOS strings
    MacAddress,         // MAC address vendor prefix
    DiskSerial,         // Disk serial number
    HostnamePattern,    // Hostname pattern detection
};

/// Konfigurasi anti-detection untuk satu artefak
struct AntiDetectConfig {
    VMArtifactType type;
    bool enabled = true;

    /// Nilai custom untuk mengganti artefak (type-dependent)
    /// - CPUIDSignature: string 12 char (mis. "GenuineIntel")
    /// - BIOSVendor: string (mis. "American Megatrends Inc.")
    /// - SystemVendor: string (mis. "ASUSTeK Computer INC.")
    /// - HypervisorBit: "0" atau "1"
    std::string customValue;
};

/// Hasil penerapan anti-detection
struct AntiDetectResult {
    bool success = false;
    std::string errorMessage;

    /// Artefak yang berhasil dimodifikasi
    std::vector<VMArtifactType> patchedArtifacts;

    /// Artefak yang gagal dimodifikasi
    struct FailedPatch {
        VMArtifactType type;
        std::string reason;
    };
    std::vector<FailedPatch> failedPatches;

    /// Detail perubahan (untuk logging/debug)
    std::vector<std::string> changes;
};

// ── QemuAntiDetect Wrapper ─────────────────────────────────────

/// Wrapper untuk anti-detection mode dalam emulation.
///
/// Fungsi utama:
/// 1. Konfigurasi environment emulasi untuk menyembunyikan artefak VM
/// 2. Modifikasi register/memory sebelum eksekusi untuk anti-detection
/// 3. Validasi bahwa artefak berhasil disembunyikan
///
/// Source: zhaodice/qemu-anti-detection (patches untuk QEMU 6.2.0-10.2.2)
class QemuAntiDetect {
public:
    QemuAntiDetect();
    ~QemuAntiDetect();

    // Non-copyable
    QemuAntiDetect(const QemuAntiDetect&) = delete;
    QemuAntiDetect& operator=(const QemuAntiDetect&) = delete;

    /// Konfigurasi anti-detection untuk satu artefak
    /// @param config  konfigurasi artefak
    /// @return true jika berhasil dikonfigurasi
    bool configure(const AntiDetectConfig& config);

    /// Konfigurasi default untuk menyembunyikan semua artefak VM
    /// @param arch  arsitektur target (ARM64, x86, dll)
    void configureDefaults(int arch);

    /// Terapkan anti-detection ke register state sebelum eksekusi
    /// Dipanggil SEBELUM UnicornEngine::execute()
    ///
    /// @param registers  map register name -> value (akan dimodifikasi in-place)
    /// @return hasil penerapan
    AntiDetectResult applyToRegisters(
        std::unordered_map<std::string, uint64_t>& registers
    ) const;

    /// Terapkan anti-detection ke memory state sebelum eksekusi
    /// Modifikasi memory region yang berisi artefak (DMI, BIOS strings, dll)
    ///
    /// @param memory     raw memory bytes (akan dimodifikasi in-place)
    /// @param baseAddr   base address dari memory region ini
    /// @return hasil penerapan
    AntiDetectResult applyToMemory(
        std::vector<uint8_t>& memory,
        uint64_t baseAddr
    ) const;

    /// Validasi apakah artefak VM masih terdeteksi
    /// @param registers  register state setelah eksekusi
    /// @param memory     memory state setelah eksekusi
    /// @param baseAddr   base address
    /// @return list artefak yang masih terdeteksi (kosong = aman)
    std::vector<VMArtifactType> validateDetection(
        const std::unordered_map<std::string, uint64_t>& registers,
        const std::vector<uint8_t>& memory,
        uint64_t baseAddr
    ) const;

    /// Apakah anti-detection aktif
    bool isEnabled() const;

    /// Ambil semua konfigurasi yang sudah di-set
    const std::vector<AntiDetectConfig>& getConfigs() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace omnibyte::hydradis
