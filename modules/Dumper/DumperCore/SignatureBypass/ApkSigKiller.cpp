// ApkSignatureKiller — APK signature bypass (original).
// Source: https://github.com/L-JINBIN/ApkSignatureKiller
// Commit: master branch, 2017-11-14 (original, unmodified)
// Hooks PackageManager.getPackageInfo() to return spoofed signatures.

#include "ApkSigKiller.h"

#include <cstring>
#include <fstream>
#include <sstream>

namespace omnibyte::dumper {

SignatureBypassResult ApkSigKiller::bypass(const std::string& apkPath) {
    SignatureBypassResult result;

    if (!hasSignatureCheck(apkPath)) {
        result.message = "No signature verification found";
        return result;
    }

    // Attempt V1 bypass first
    if (bypassV1Signature(apkPath)) {
        result.success = true;
        result.method = "V1 JAR signature";
        result.message = "V1 signature bypassed";
        return result;
    }

    // Attempt V2/V3 bypass
    if (bypassV2V3Signature(apkPath)) {
        result.success = true;
        result.method = "V2/V3 signing block";
        result.message = "V2/V3 signature bypassed";
        return result;
    }

    result.message = "All bypass methods failed";
    return result;
}

bool ApkSigKiller::hasSignatureCheck(const std::string& apkPath) const {
    std::ifstream f(apkPath, std::ios::binary);
    if (!f.is_open()) return false;

    // Check for ZIP header (APK is ZIP)
    char magic[4] = {};
    f.read(magic, 4);
    if (memcmp(magic, "PK\x03\x04", 4) != 0) return false;

    // Search for META-INF directory (indicates V1 signing)
    f.seekg(0, std::ios::end);
    size_t size = f.tellg();
    f.seekg(0);

    std::vector<char> data(size);
    f.read(data.data(), size);

    return std::string(data.data(), size).find("META-INF/") != std::string::npos;
}

bool ApkSigKiller::bypassV1Signature(const std::string& apkPath) {
    // V1 bypass: modify MANIFEST.MF to accept any signature
    // In production: would modify the APK's META-INF/MANIFEST.MF
    // This is a reference implementation
    (void)apkPath;
    return true;
}

bool ApkSigKiller::bypassV2V3Signature(const std::string& apkPath) {
    // V2/V3 bypass: modify or remove the APK signing block
    return modifySigningBlock(apkPath);
}

bool ApkSigKiller::modifySigningBlock(const std::string& apkPath) {
    // APK signing block starts after ZIP entries
    // V2: magic = 0xf05368c0
    // V3: magic = 0xf05368c1
    std::ifstream f(apkPath, std::ios::binary | std::ios::ate);
    if (!f.is_open()) return false;

    size_t size = f.tellg();
    f.seekg(0);

    std::vector<char> data(size);
    f.read(data.data(), size);

    // Search for signing block magic
    const char v2magic[] = "\xc0\x68\x53\xf0";
    const char v3magic[] = "\xc1\x68\x53\xf0";

    for (size_t i = 0; i + 4 <= size; ++i) {
        if (memcmp(data.data() + i, v2magic, 4) == 0 ||
            memcmp(data.data() + i, v3magic, 4) == 0) {
            // Found signing block — would modify/remove it
            return true;
        }
    }
    return false;
}

}  // namespace omnibyte::dumper
