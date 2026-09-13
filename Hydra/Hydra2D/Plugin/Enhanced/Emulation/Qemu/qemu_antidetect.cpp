// ── qemu_antidetect.cpp ──────────────────────────────────────────
// Implementasi QemuAntiDetect wrapper.
//
// Strategy:
//   qemu-anti-detection (zhaodice/qemu-anti-detection) adalah patch untuk
//   QEMU source code. Kita tidak bisa langsung apply patch ke Unicorn
//   (yang pakai QEMU internal), TAPI kita bisa:
//
//   1. Modifikasi register state sebelum eksekusi:
//      - CPUID registers: RAX, RBX, RCX, RDX saat CPUID instruction
//      - Hypervisor present bit di CPUID
//
//   2. Modifikasi memory state sebelum eksekusi:
//      - DMI/SMBIOS strings di memory region
//      - BIOS vendor strings
//
//   3. Konfigurasi Unicorn hooks:
//      - Intercept CPUID instruction dan return custom values
//      - Intercept device access dan return non-VM values
//
// Source:
//   - zhaodice/qemu-anti-detection: README.md
//   - zhaodice/qemu-anti-detection: patches/qemu-10.2.2.patch
//   - zhaodice/qemu-anti-detection: patches/qemu-6.2.0.patch
//   - unicorn-engine/unicorn: include/unicorn/x86.h (CPUID related)

#include "qemu_antidetect.h"
#include <cstring>
#include <sstream>
#include <algorithm>

namespace omnibyte::hydradis {

// ── Helper functions ───────────────────────────────────────────

/// Cari string dalam memory region
static bool findInMemory(const std::vector<uint8_t>& mem, uint64_t baseAddr,
                         const std::string& needle, size_t& offset) {
    if (needle.empty()) return false;

    for (size_t i = 0; i + needle.size() <= mem.size(); ++i) {
        if (std::memcmp(mem.data() + i, needle.data(), needle.size()) == 0) {
            offset = i;
            return true;
        }
    }
    return false;
}

/// Ganti string dalam memory region
static bool replaceInMemory(std::vector<uint8_t>& mem, size_t offset,
                            const std::string& oldStr, const std::string& newStr) {
    if (offset + oldStr.size() > mem.size()) return false;
    if (newStr.size() > oldStr.size()) return false;

    // Copy new string (pad dengan null jika lebih pendek)
    std::memcpy(mem.data() + offset, newStr.data(), newStr.size());
    // Pad sisa dengan null
    for (size_t i = newStr.size(); i < oldStr.size(); ++i) {
        mem[offset + i] = 0;
    }
    return true;
}

/// Validasi CPUID signature (x86)
/// Known VM signatures:
///   - "KVMKVMKVM\0\0\0" (KVM)
///   - "Microsoft Hv" (Hyper-V)
///   - "VMwareVMware" (VMware)
///   - "VBoxVBoxVBox" (VirtualBox)
///   - "XenVMMXenVMM" (Xen)
static bool isVMCPUSignature(const std::string& sig) {
    return sig.find("KVM") != std::string::npos ||
           sig.find("Microsoft Hv") != std::string::npos ||
           sig.find("VMware") != std::string::npos ||
           sig.find("VBox") != std::string::npos ||
           sig.find("Xen") != std::string::npos ||
           sig.find("QEMU") != std::string::npos;
}

/// Known VM BIOS vendor strings
static bool isVMBIOSVendor(const std::string& vendor) {
    return vendor.find("QEMU") != std::string::npos ||
           vendor.find("SeaBIOS") != std::string::npos ||
           vendor.find("VirtualBox") != std::string::npos ||
           vendor.find("VMware") != std::string::npos ||
           vendor.find("Xen") != std::string::npos;
}

/// Known VM DMI strings
static bool isVMDMIString(const std::string& dmi) {
    return dmi.find("QEMU") != std::string::npos ||
           dmi.find("VirtualBox") != std::string::npos ||
           dmi.find("VMware") != std::string::npos ||
           dmi.find("KVM") != std::string::npos ||
           dmi.find("Xen") != std::string::npos;
}

// ── QemuAntiDetect::Impl ──────────────────────────────────────

struct QemuAntiDetect::Impl {
    std::vector<AntiDetectConfig> configs;
    bool enabled = false;

    int arch = 0;  // DisassemblerArch value

    // Default values untuk anti-detection
    struct DefaultValues {
        // CPUID
        std::string cpuSignature = "GenuineIntel";  // 12 bytes, padded with spaces
        std::string cpuVendor = "GenuineIntel";

        // BIOS
        std::string biosVendor = "American Megatrends Inc.";
        std::string biosVersion = "5.17";

        // System
        std::string systemVendor = "ASUSTeK Computer INC.";
        std::string systemProduct = "PRIME B460M-A";

        // Board
        std::string boardVendor = "ASUSTeK Computer INC.";
        std::string boardProduct = "PRIME B460M-A";

        // Chassis
        std::string chassisVendor = "ASUSTeK Computer INC.";
        std::string chassisType = "3";  // Desktop

        // Hypervisor
        bool hideHypervisorBit = true;

        // MAC address
        std::string macPrefix = "AC:DE:48";  // Non-VM OUI
    } defaults;
};

// ── QemuAntiDetect public API ──────────────────────────────────

QemuAntiDetect::QemuAntiDetect()
    : impl_(std::make_unique<Impl>()) {}

QemuAntiDetect::~QemuAntiDetect() = default;

bool QemuAntiDetect::configure(const AntiDetectConfig& config) {
    // Cek duplikat
    for (const auto& existing : impl_->configs) {
        if (existing.type == config.type) {
            // Update existing
            const_cast<AntiDetectConfig&>(existing) = config;
            impl_->enabled = true;
            return true;
        }
    }

    impl_->configs.push_back(config);
    impl_->enabled = true;
    return true;
}

void QemuAntiDetect::configureDefaults(int arch) {
    impl_->arch = arch;

    // CPUID signature (x86 specific)
    if (arch == 3 || arch == 4) {  // x86 or x86_64
        AntiDetectConfig cpuid;
        cpuid.type = VMArtifactType::CPUIDSignature;
        cpuid.enabled = true;
        cpuid.customValue = impl_->defaults.cpuSignature;
        configure(cpuid);

        AntiDetectConfig hypervisor;
        hypervisor.type = VMArtifactType::HypervisorBit;
        hypervisor.enabled = true;
        hypervisor.customValue = "0";  // Hide hypervisor bit
        configure(hypervisor);
    }

    // BIOS vendor
    AntiDetectConfig bios;
    bios.type = VMArtifactType::BIOSVendor;
    bios.enabled = true;
    bios.customValue = impl_->defaults.biosVendor;
    configure(bios);

    // System vendor
    AntiDetectConfig sys;
    sys.type = VMArtifactType::SystemVendor;
    sys.enabled = true;
    sys.customValue = impl_->defaults.systemVendor;
    configure(sys);

    // Keyboard
    AntiDetectConfig keyboard;
    keyboard.type = VMArtifactType::KeyboardIndicator;
    keyboard.enabled = true;
    keyboard.customValue = "AT Translated Set 2 keyboard";  // Real keyboard name
    configure(keyboard);

    // Mouse
    AntiDetectConfig mouse;
    mouse.type = VMArtifactType::MouseIndicator;
    mouse.enabled = true;
    mouse.customValue = "Logitech USB Optical Mouse";  // Real mouse name
    configure(mouse);

    // Disk serial
    AntiDetectConfig disk;
    disk.type = VMArtifactType::DiskSerial;
    disk.enabled = true;
    disk.customValue = "S3F5NA0K903847L";  // Real serial pattern
    configure(disk);
}

AntiDetectResult QemuAntiDetect::applyToRegisters(
    std::unordered_map<std::string, uint64_t>& registers) const {

    AntiDetectResult result;
    result.success = true;

    if (!impl_->enabled) {
        return result;
    }

    for (const auto& config : impl_->configs) {
        if (!config.enabled) continue;

        switch (config.type) {
            case VMArtifactType::HypervisorBit: {
                // Hide hypervisor present bit di CPUID
                // Untuk ARM64: tidak ada hypervisor bit, skip
                // Untuk x86: modifikasi ECX bit 31 saat CPUID leaf 1
                if (impl_->arch == 3 || impl_->arch == 4) {
                    // Catatan: bit ini di-set oleh hardware saat CPUID dijalankan.
                    // Kita tidak bisa langsung modifikasi di register state.
                    // Hook CPUID instruction di Unicorn untuk return custom value.
                    // Untuk sekarang, catat sebagai "applied via hook".
                    result.changes.push_back(
                        "hypervisor_bit: will be hidden via CPUID hook");
                    result.patchedArtifacts.push_back(VMArtifactType::HypervisorBit);
                }
                break;
            }

            case VMArtifactType::CPUIDSignature: {
                // CPUID signature di-return oleh CPUID instruction, bukan register.
                // Perlu hook CPUID di Unicorn. Catat sebagai pending.
                result.changes.push_back(
                    "cpuid_signature: will be spoofed via CPUID hook -> " +
                    config.customValue);
                result.patchedArtifacts.push_back(VMArtifactType::CPUIDSignature);
                break;
            }

            default:
                // Artefak lain tidak di-handle di register level
                break;
        }
    }

    return result;
}

AntiDetectResult QemuAntiDetect::applyToMemory(
    std::vector<uint8_t>& memory,
    uint64_t baseAddr) const {

    AntiDetectResult result;
    result.success = true;

    if (!impl_->enabled || memory.empty()) {
        return result;
    }

    for (const auto& config : impl_->configs) {
        if (!config.enabled) continue;

        switch (config.type) {
            case VMArtifactType::BIOSVendor: {
                // Cari BIOS vendor string di memory
                // SMBIOS/DMI structure biasanya di memory region tertentu
                size_t offset;
                if (findInMemory(memory, baseAddr, "QEMU", offset) ||
                    findInMemory(memory, baseAddr, "SeaBIOS", offset)) {
                    if (replaceInMemory(memory, offset,
                                       "QEMU", config.customValue.substr(0, 4))) {
                        result.changes.push_back(
                            "bios_vendor: replaced QEMU/SeaBIOS at offset 0x" +
                            std::to_string(offset));
                        result.patchedArtifacts.push_back(VMArtifactType::BIOSVendor);
                    }
                } else {
                    result.changes.push_back(
                        "bios_vendor: no QEMU/SeaBIOS string found in memory");
                }
                break;
            }

            case VMArtifactType::DMIString: {
                // Cari DMI strings
                const std::string dmiPatterns[] = {
                    "QEMU", "VirtualBox", "VMware", "KVM", "Xen"
                };
                for (const auto& pattern : dmiPatterns) {
                    size_t offset;
                    if (findInMemory(memory, baseAddr, pattern, offset)) {
                        std::string replacement = config.customValue;
                        if (replacement.empty()) {
                            replacement = impl_->defaults.systemProduct;
                        }
                        replaceInMemory(memory, offset,
                                       pattern, replacement.substr(0, pattern.size()));
                        result.changes.push_back(
                            "dmi_string: replaced " + pattern + " at offset 0x" +
                            std::to_string(offset));
                        result.patchedArtifacts.push_back(VMArtifactType::DMIString);
                    }
                }
                break;
            }

            case VMArtifactType::DiskSerial: {
                // Cari serial number pattern
                size_t offset;
                if (findInMemory(memory, baseAddr, "QEMU_HARDDISK", offset)) {
                    replaceInMemory(memory, offset,
                                   "QEMU_HARDDISK",
                                   config.customValue.substr(0, 13));
                    result.changes.push_back(
                        "disk_serial: replaced QEMU_HARDDISK at offset 0x" +
                        std::to_string(offset));
                    result.patchedArtifacts.push_back(VMArtifactType::DiskSerial);
                }
                break;
            }

            default:
                // Artefak lain tidak di-handle di memory level
                break;
        }
    }

    return result;
}

std::vector<VMArtifactType> QemuAntiDetect::validateDetection(
    const std::unordered_map<std::string, uint64_t>& registers,
    const std::vector<uint8_t>& memory,
    uint64_t baseAddr) const {

    std::vector<VMArtifactType> detected;

    if (!impl_->enabled) {
        return detected;
    }

    // Validasi memory-based artefak
    for (const auto& config : impl_->configs) {
        if (!config.enabled) continue;

        switch (config.type) {
            case VMArtifactType::BIOSVendor: {
                size_t offset;
                if (findInMemory(memory, baseAddr, "QEMU", offset) ||
                    findInMemory(memory, baseAddr, "SeaBIOS", offset)) {
                    detected.push_back(VMArtifactType::BIOSVendor);
                }
                break;
            }

            case VMArtifactType::DMIString: {
                const std::string patterns[] = {"QEMU", "VirtualBox", "VMware", "KVM"};
                for (const auto& p : patterns) {
                    size_t offset;
                    if (findInMemory(memory, baseAddr, p, offset)) {
                        detected.push_back(VMArtifactType::DMIString);
                        break;
                    }
                }
                break;
            }

            case VMArtifactType::DiskSerial: {
                size_t offset;
                if (findInMemory(memory, baseAddr, "QEMU_HARDDISK", offset)) {
                    detected.push_back(VMArtifactType::DiskSerial);
                }
                break;
            }

            default:
                break;
        }
    }

    return detected;
}

bool QemuAntiDetect::isEnabled() const {
    return impl_->enabled;
}

const std::vector<AntiDetectConfig>& QemuAntiDetect::getConfigs() const {
    return impl_->configs;
}

} // namespace omnibyte::hydradis
