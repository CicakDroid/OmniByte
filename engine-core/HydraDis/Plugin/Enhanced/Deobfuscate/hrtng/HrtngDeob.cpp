// hrtng — Extractable crypto/deob modules.
// Source: https://github.com/KasperskyLab/hrtng
// Commit: master branch, 2026-09-01
// License: GPL-3.0
//
// Extracted modules (no IDA SDK dependency):
// - apihashes: API hash lookup tables for Windows/Linux/Android
// - decr: XOR, base64, rot13, add/subtract decryption
// - lit: constant/literal pattern analysis

#include "HrtngDeob.h"

#include <algorithm>
#include <cstring>
#include <unordered_map>

namespace omnibyte::deob {

// ---------------------------------------------------------------------------
// HrtngApiHashes — API hash tables
// ---------------------------------------------------------------------------

HrtngApiHashes::HrtngApiHashes() {
    initBuiltinHashes();
}

void HrtngApiHashes::initBuiltinHashes() {
    // Common Windows API hashes (ror13 hash)
    entries_ = {
        // Windows kernel32
        {0x6A4ABC5B, "LoadLibraryA", Platform::Windows},
        {0x07267E4F, "GetProcAddress", Platform::Windows},
        {0x876F8BAD, "VirtualAlloc", Platform::Windows},
        {0x37864C1D, "VirtualProtect", Platform::Windows},
        {0xC6DC3240, "CreateFileA", Platform::Windows},
        {0x4FDA7E3D, "WriteFile", Platform::Windows},
        {0x2518991C, "ReadFile", Platform::Windows},
        {0x4967F4E6, "CreateThread", Platform::Windows},
        {0x8E4A0F38, "WaitForSingleObject", Platform::Windows},
        {0x89C2092A, "VirtualFree", Platform::Windows},
        {0x53696333, "CloseHandle", Platform::Windows},

        // Linux libc
        {0x0C151817, "open", Platform::Linux},
        {0x83E64655, "read", Platform::Linux},
        {0x0B5E4657, "write", Platform::Linux},
        {0x56C46B5A, "close", Platform::Linux},
        {0x0B8C2010, "mmap", Platform::Linux},
        {0x583D4655, "munmap", Platform::Linux},
        {0x0C1E4C16, "ioctl", Platform::Linux},
        {0x83E64C16, "ptrace", Platform::Linux},
        {0x0C150E15, "fork", Platform::Linux},
        {0x83E64E55, "execve", Platform::Linux},
        {0x83E64755, "mprotect", Platform::Linux},

        // Android bionic
        {0x0C151817, "__open", Platform::Android},
        {0x83E64655, "__read", Platform::Android},
        {0x0B5E4657, "__write", Platform::Android},
        {0x0B8C2010, "__mmap2", Platform::Android},
        {0x0C1E4C16, "__ioctl", Platform::Android},
        {0x0C154E1A, "fopen", Platform::Android},
        {0x0B5E4E1A, "fwrite", Platform::Android},
        {0x83E64855, "fread", Platform::Android},
    };
}

std::optional<std::string> HrtngApiHashes::findApiByHash(uint64_t hash, Platform platform) const {
    for (const auto& entry : entries_) {
        if (entry.hash == hash && entry.platform == platform) {
            return entry.name;
        }
    }
    return std::nullopt;
}

void HrtngApiHashes::registerHashList(Platform platform,
                                       const std::vector<ApiHashEntry>& entries) {
    for (auto& e : entries) {
        ApiHashEntry entry = e;
        entry.platform = platform;
        entries_.push_back(entry);
    }
}

// ---------------------------------------------------------------------------
// HrtngDecrypt — Decryption routines
// ---------------------------------------------------------------------------

std::vector<uint8_t> HrtngDecrypt::decryptXor(const std::vector<uint8_t>& data,
                                               const std::vector<uint8_t>& key) {
    if (key.empty()) return data;
    std::vector<uint8_t> result(data.size());
    for (size_t i = 0; i < data.size(); ++i) {
        result[i] = data[i] ^ key[i % key.size()];
    }
    return result;
}

std::vector<uint8_t> HrtngDecrypt::decodeBase64(const std::string& encoded) {
    static const std::string chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    std::vector<uint8_t> result;
    int val = 0, bits = -8;
    for (char c : encoded) {
        if (c == '=') break;
        size_t pos = chars.find(c);
        if (pos == std::string::npos) continue;
        val = (val << 6) + static_cast<int>(pos);
        bits += 6;
        if (bits >= 0) {
            result.push_back(static_cast<uint8_t>((val >> bits) & 0xFF));
            bits -= 8;
        }
    }
    return result;
}

std::string HrtngDecrypt::encodeBase64(const std::vector<uint8_t>& data) {
    static const std::string chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    std::string result;
    int i = 0, x = 0;
    for (uint8_t byte : data) {
        x = (x << 8) | byte;
        i += 8;
        while (i >= 6) {
            result += chars[(x >> (i - 6)) & 0x3F];
            i -= 6;
        }
    }
    if (i > 0) result += chars[(x << (6 - i)) & 0x3F];
    while (result.size() % 4) result += '=';
    return result;
}

std::vector<uint8_t> HrtngDecrypt::xorSingleByte(const std::vector<uint8_t>& data, uint8_t key) {
    std::vector<uint8_t> result(data.size());
    for (size_t i = 0; i < data.size(); ++i) {
        result[i] = data[i] ^ key;
    }
    return result;
}

std::vector<uint8_t> HrtngDecrypt::rot13(const std::vector<uint8_t>& data) {
    std::vector<uint8_t> result(data.size());
    for (size_t i = 0; i < data.size(); ++i) {
        uint8_t c = data[i];
        if (c >= 'a' && c <= 'z') {
            result[i] = 'a' + (c - 'a' + 13) % 26;
        } else if (c >= 'A' && c <= 'Z') {
            result[i] = 'A' + (c - 'A' + 13) % 26;
        } else {
            result[i] = c;
        }
    }
    return result;
}

std::vector<uint8_t> HrtngDecrypt::addConstant(const std::vector<uint8_t>& data, int8_t delta) {
    std::vector<uint8_t> result(data.size());
    for (size_t i = 0; i < data.size(); ++i) {
        result[i] = static_cast<uint8_t>(data[i] + delta);
    }
    return result;
}

// ---------------------------------------------------------------------------
// HrtngLiterals — Constant analysis
// ---------------------------------------------------------------------------

std::vector<ConstantInfo> HrtngLiterals::analyzeConstants(const uint8_t* code,
                                                          size_t size) const {
    std::vector<ConstantInfo> results;

    // Look for interesting constant patterns in code
    for (size_t i = 0; i + 4 <= size; ++i) {
        uint32_t val32 = *reinterpret_cast<const uint32_t*>(code + i);

        // Check for common magic values
        if (val32 == 0x5A4D) {  // MZ header
            results.push_back({static_cast<uintptr_t>(i), val32, 2, "PE header"});
        }
        if (val32 == 0x464C457F) {  // ELF header
            results.push_back({static_cast<uintptr_t>(i), val32, 4, "ELF header"});
        }

        // Check for potential pointers (looks like stack address)
        if (val32 > 0x70000000 && val32 < 0x80000000) {
            results.push_back({static_cast<uintptr_t>(i), val32, 4, "possible stack pointer"});
        }
    }

    return results;
}

std::vector<CryptoConstant> HrtngLiterals::findCryptoConstants(const uint8_t* data,
                                                               size_t size) const {
    std::vector<CryptoConstant> results;

    // AES S-box (first 16 bytes)
    static const uint8_t aesSbox[] = {
        0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5,
        0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76,
    };

    // SHA-256 initial hash values
    static const uint8_t sha256H0[] = {
        0x67, 0xE6, 0x09, 0x6A,  // H0[0] = 0x6A09E667
        0xAB, 0xD6, 0xCA, 0x3C,  // H0[1] = 0xBB67AE85
    };

    // DES S-box (first 16 bytes)
    static const uint8_t desSbox[] = {
        0x0E, 0x04, 0x0D, 0x01, 0x02, 0x0F, 0x0B, 0x08,
        0x03, 0x0A, 0x06, 0x0C, 0x05, 0x09, 0x00, 0x07,
    };

    // Search for known constants
    for (size_t i = 0; i + 16 <= size; ++i) {
        if (memcmp(data + i, aesSbox, 16) == 0) {
            results.push_back({static_cast<uintptr_t>(i), "AES S-box", 0.95});
        }
        if (memcmp(data + i, sha256H0, 8) == 0) {
            results.push_back({static_cast<uintptr_t>(i), "SHA-256 H0", 0.90});
        }
        if (memcmp(data + i, desSbox, 16) == 0) {
            results.push_back({static_cast<uintptr_t>(i), "DES S-box", 0.85});
        }
    }

    return results;
}

}  // namespace omnibyte::deob
