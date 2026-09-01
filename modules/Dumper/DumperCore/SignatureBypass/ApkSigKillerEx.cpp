// ApkSignatureKillerEx — Extended APK signature bypass.
// Source: https://github.com/L-JINBIN/ApkSignatureKillerEx
// Commit: master branch, 2023-01-25
// Native-level APK signing block manipulation.

#include "ApkSigKillerEx.h"

#include <cstring>
#include <fstream>

namespace omnibyte::dumper {

static constexpr uint32_t APK_V2_MAGIC = 0xf05368c0;
static constexpr uint32_t APK_V3_MAGIC = 0xf05368c1;

SignatureBypassResult ApkSigKillerEx::bypass(const std::string& apkPath) {
    SignatureBypassResult result;

    if (removeSigningBlock(apkPath)) {
        result.success = true;
        result.method = "Ex: signing block removal";
        result.message = "APK signing block removed";
        return result;
    }

    if (patchSigningBlock(apkPath)) {
        result.success = true;
        result.method = "Ex: signing block patch";
        result.message = "APK signing block patched";
        return result;
    }

    result.message = "Ex: all bypass methods failed";
    return result;
}

bool ApkSigKillerEx::hasSignatureCheck(const std::string& apkPath) const {
    std::ifstream f(apkPath, std::ios::binary | std::ios::ate);
    if (!f.is_open()) return false;

    size_t size = f.tellg();
    f.seekg(0);

    std::vector<char> data(size);
    f.read(data.data(), size);

    return findSigningBlockOffset(data) != 0;
}

bool ApkSigKillerEx::patchSigningBlock(const std::string& apkPath) {
    std::ifstream f(apkPath, std::ios::binary | std::ios::ate);
    if (!f.is_open()) return false;

    size_t size = f.tellg();
    f.seekg(0);

    std::vector<char> data(size);
    f.read(data.data(), size);

    size_t blockOffset = findSigningBlockOffset(data);
    if (blockOffset == 0) return false;

    // Patch the signing block magic to invalidate it
    // V2 → 0x00000000 (invalid magic)
    memset(data.data() + blockOffset, 0, 4);

    // Write back
    std::ofstream out(apkPath, std::ios::binary | std::ios::trunc);
    if (!out.is_open()) return false;
    out.write(data.data(), size);
    return true;
}

bool ApkSigKillerEx::removeSigningBlock(const std::string& apkPath) {
    std::ifstream f(apkPath, std::ios::binary | std::ios::ate);
    if (!f.is_open()) return false;

    size_t size = f.tellg();
    f.seekg(0);

    std::vector<char> data(size);
    f.read(data.data(), size);

    size_t blockOffset = findSigningBlockOffset(data);
    if (blockOffset == 0) return false;

    // Remove everything from block offset to end
    std::vector<char> trimmed(data.begin(), data.begin() + blockOffset);

    std::ofstream out(apkPath, std::ios::binary | std::ios::trunc);
    if (!out.is_open()) return false;
    out.write(trimmed.data(), trimmed.size());
    return true;
}

bool ApkSigKillerEx::spoofCertificateHash(const std::string& apkPath,
                                            const std::vector<uint8_t>& fakeHash) {
    // Would modify the certificate hash in the signing block
    (void)apkPath;
    (void)fakeHash;
    return false;
}

size_t ApkSigKillerEx::findSigningBlockOffset(const std::vector<char>& data) const {
    // Search backwards for signing block magic
    for (size_t i = data.size() - 4; i >= 4; --i) {
        uint32_t magic;
        memcpy(&magic, data.data() + i, 4);
        if (magic == APK_V2_MAGIC || magic == APK_V3_MAGIC) {
            return i;
        }
    }
    return 0;
}

uint32_t ApkSigKillerEx::calculateCRC(const std::vector<char>& data,
                                        size_t start, size_t end) const {
    // CRC32 calculation for ZIP compatibility
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = start; i < end; ++i) {
        crc ^= static_cast<uint8_t>(data[i]);
        for (int j = 0; j < 8; ++j) {
            crc = (crc >> 1) ^ (0xEDB88320 & (-(crc & 1)));
        }
    }
    return ~crc;
}

}  // namespace omnibyte::dumper
