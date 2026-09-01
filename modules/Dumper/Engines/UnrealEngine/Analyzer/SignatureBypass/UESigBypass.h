#pragma once
// UniversalSigBypasser — Unreal Engine signature bypass.
// Source: https://github.com/rm-NoobInCoding/UniversalSigBypasser

#include <cstdint>
#include <string>
#include <vector>

namespace omnibyte::dumper {

/// UE signature bypass result.
struct UEBypassResult {
    bool success = false;
    std::string method;
    std::string message;
    uintptr_t patchedAddress = 0;
};

/// Unreal Engine signature verification bypass.
/// Patches UE's signature check functions at runtime.
class UESigBypass {
public:
    UESigBypass() = default;
    ~UESigBypass() = default;

    /// Initialize and find UE module in current process.
    bool init();

    /// Bypass all UE signature checks.
    UEBypassResult bypassAll();

    /// Find UE signature verification function.
    uintptr_t findSignatureCheckFunction() const;

    /// Patch the signature check function.
    bool patchSignatureCheck(uintptr_t address);

    /// Check if UE signature check is active.
    bool isSignatureCheckActive() const;

private:
    uintptr_t ueBase_ = 0;
    size_t ueSize_ = 0;

    /// Known UE signature check patterns (ARM64).
    static const std::vector<std::vector<uint8_t>> signatureCheckPatterns_;
};

}  // namespace omnibyte::dumper
