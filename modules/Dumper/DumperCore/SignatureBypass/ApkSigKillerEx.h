#pragma once
// ApkSignatureKillerEx — Extended APK signature bypass (C implementation).
// Source: https://github.com/L-JINBIN/ApkSignatureKillerEx (No license specified)

#include "ApkSigKiller.h"
#include <cstdint>
#include <string>
#include <vector>

namespace omnibyte::dumper {

/// Extended APK signature bypass with additional bypass methods.
class ApkSigKillerEx : public ISignatureBypass {
public:
    ApkSigKillerEx() = default;
    ~ApkSigKillerEx() override = default;

    SignatureBypassResult bypass(const std::string& apkPath) override;
    bool hasSignatureCheck(const std::string& apkPath) const override;
    std::string getName() const override { return "ApkSigKillerEx"; }

    /// Patch APK signing block (V2/V3) at native level.
    bool patchSigningBlock(const std::string& apkPath);

    /// Remove signing block entirely.
    bool removeSigningBlock(const std::string& apkPath);

    /// Spoof certificate hash in signing block.
    bool spoofCertificateHash(const std::string& apkPath,
                              const std::vector<uint8_t>& fakeHash);

private:
    /// Find signing block offset in APK.
    size_t findSigningBlockOffset(const std::vector<char>& data) const;

    /// Calculate new CRC after modification.
    uint32_t calculateCRC(const std::vector<char>& data, size_t start, size_t end) const;
};

}  // namespace omnibyte::dumper
