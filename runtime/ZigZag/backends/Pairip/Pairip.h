#pragma once
// Pairip — Google Play Integrity / Play Protect bypass.
// Technique: GMS property spoofing + DroidGuard response interception.
//
// Sources:
//   - KOWX712/PlayIntegrityFix (GPL-3.0, 3.8k stars)
//     https://github.com/KOWX712/PlayIntegrityFix
//     Zygisk module that spoofs device properties and keybox to pass
//     MEETS_DEVICE_INTEGRITY verdict.
//   - dpejoh/specter (GPL-3.0)
//     https://github.com/dpejoh/specter
//     Unified Play Integrity and root hiding stack.
//
// Bypasses:
//   - MEETS_BASIC_INTEGRITY: device not rooted/unlocked
//   - MEETS_DEVICE_INTEGRITY: certified device with valid keybox
//   - MEETS_STRONG_INTEGRITY: hardware-backed attestation
//
// Implementation: Userspace property spoofing via __system_property_get hook
// and GMS process detection.

#include "IStealthBackend.h"

#include <string>
#include <unordered_map>

namespace omnibyte::runtime::backends {

/// Play Integrity / Play Protect bypass via GMS property spoofing.
/// Intercepts system property queries and GMS DroidGuard responses.
class Pairip : public IStealthBackend {
public:
    Pairip() = default;
    ~Pairip() = default;

    std::string name() const override { return "Pairip"; }
    bool isAvailable() const override;
    bool hide(pid_t pid) override;
    bool unhide(pid_t pid) override;
    bool bypassPtraceScope() override;
    bool bypassSelinuxDenial() override;

    // --- Pairip-specific API ---

    /// Spoof device properties for Play Integrity attestation.
    /// Hooks __system_property_get to return safe values for:
    ///   - ro.build.fingerprint
    ///   - ro.build.version.security_patch
    ///   - ro.boot.flash.locked
    ///   - ro.build.type
    bool spoofDeviceProperties();

    /// Hook GMS DroidGuard to return MEETS_DEVICE_INTEGRITY.
    /// Intercepts the integrity check response from com.google.android.gms.
    bool hookDroidGuardResponse();

    /// Spoof keybox attestation for hardware-backed integrity.
    /// Requires valid keybox (from TrickyStore/TEESimulator).
    bool spoofKeyboxAttestation();

    /// Run all bypasses: property spoofing + DroidGuard hook.
    bool bypassAll();

private:
    bool active_ = false;

    /// Property spoofing map: original -> spoofed value.
    std::unordered_map<std::string, std::string> spoofedProps_;

    /// Initialize default spoofed properties for Play Integrity.
    void initDefaultSpoofs();

    /// Hook libc __system_property_get via dlsym + PLT patching.
    bool hookPropertyGet();

    /// Check if GMS process is running.
    bool isGmsRunning() const;
};

} // namespace omnibyte::runtime::backends
