#pragma once
// ApkSignatureKiller — APK signature bypass (original).
// Source: https://github.com/L-JINBIN/ApkSignatureKiller (No license specified)

#include <cstdint>
#include <string>
#include <vector>

namespace omnibyte::dumper {

/// Signature bypass result.
struct SignatureBypassResult {
    bool success = false;
    std::string method;
    std::string message;
};

/// Base class for signature verification bypass.
class ISignatureBypass {
public:
    virtual ~ISignatureBypass() = default;

    /// Perform signature bypass on the target APK.
    virtual SignatureBypassResult bypass(const std::string& apkPath) = 0;

    /// Check if signature verification is present.
    virtual bool hasSignatureCheck(const std::string& apkPath) const = 0;

    /// Get the name of this bypass method.
    virtual std::string getName() const = 0;
};

/// APK signature bypass adapted from ApkSignatureKiller.
/// Hooks PackageManager signature verification methods.
class ApkSigKiller : public ISignatureBypass {
public:
    ApkSigKiller() = default;
    ~ApkSigKiller() override = default;

    SignatureBypassResult bypass(const std::string& apkPath) override;
    bool hasSignatureCheck(const std::string& apkPath) const override;
    std::string getName() const override { return "ApkSigKiller"; }

    /// Bypass V1 (JAR) signature verification.
    bool bypassV1Signature(const std::string& apkPath);

    /// Bypass V2/V3 signature verification.
    bool bypassV2V3Signature(const std::string& apkPath);

private:
    /// Find and modify APK signing block.
    bool modifySigningBlock(const std::string& apkPath);
};

}  // namespace omnibyte::dumper
