#pragma once
// Crc — APK CRC/checksum bypass.
// Technique: Hook PackageManager.getPackageInfo() to return spoofed signatures
// and checksums, defeating APK integrity verification.
//
// Sources:
//   - aimardcr/APKKiller (458 stars)
//     https://github.com/aimardcr/APKKiller
//     Bypasses APK signature verification and integrity checks using JNI
//     and reflection to bypass Hidden API Restriction.
//   - L-JINBIN/ApkSignatureKiller (971 stars)
//     https://github.com/L-JINBIN/ApkSignatureKiller
//     Hooks PackageManager.getPackageInfo() to return spoofed signature.
//   - riyadmondol2006/Android-Signature-And-Integrity-Check-Bypass (71 stars)
//     https://github.com/riyadmondol2006/Android-Signature-And-Integrity-Check-Bypass
//     Xposed hook for signature and integrity check bypass.
//
// Android Checksum Types (API 31+):
//   - TYPE_WHOLE_MERKLE_ROOT_4K_SHA256: fs-verity based (recommended)
//   - TYPE_WHOLE_MD5: deprecated, cryptographically broken
//   - TYPE_PARTIAL_MERKLE_ROOT_1M_SHA256: APK Signature Scheme V2
//   - TYPE_PARTIAL_MERKLE_ROOT_1M_SHA512: APK Signature Scheme V2
//   Reference: https://developer.android.com/reference/android/content/pm/Checksum

#include "IStealthBackend.h"

#include <string>
#include <vector>
#include <cstdint>

namespace omnibyte::runtime::backends {

/// APK CRC/checksum bypass via PackageManager hooking.
/// Defeats APK integrity verification by returning spoofed signatures.
class Crc : public IStealthBackend {
public:
    Crc() = default;
    ~Crc() = default;

    std::string name() const override { return "Crc"; }
    bool isAvailable() const override;
    bool hide(pid_t pid) override;
    bool unhide(pid_t pid) override;
    bool bypassPtraceScope() override;
    bool bypassSelinuxDenial() override;

    // --- Crc-specific API ---

    /// Hook PackageManager.getPackageInfo() to return spoofed signatures.
    /// Replaces SIGNATURE, SIGNING_CERTIFICATES, and GET_SIGNING_CERTIFICATES.
    bool hookSignatureVerification();

    /// Hook PackageManager.requestChecksums() to return valid checksums.
    /// Spoofs TYPE_WHOLE_MERKLE_ROOT_4K_SHA256 and TYPE_WHOLE_MD5.
    bool hookChecksumVerification();

    /// Spoof APK signing certificate with target package's certificate.
    /// Copies the original signing cert to fool integrity checks.
    bool spoofSigningCertificate(const char* targetPackage);

    /// Run all CRC bypasses.
    bool bypassAll();

private:
    bool active_ = false;

    /// Target package name for certificate spoofing.
    std::string targetPackage_;

    /// Original signing certificate (to restore if needed).
    std::vector<uint8_t> originalCert_;

    /// Hook JNI calls to PackageManager via ART method hooking.
    bool hookGetPackageInfo();

    /// Hook PackageManager.requestChecksums via reflection.
    bool hookRequestChecksums();
};

} // namespace omnibyte::runtime::backends
