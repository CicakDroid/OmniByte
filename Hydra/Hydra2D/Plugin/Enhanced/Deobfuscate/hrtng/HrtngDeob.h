#pragma once
// hrtng — Extractable crypto/deob modules from Kaspersky IDA plugin.
// Source: https://github.com/KasperskyLab/hrtng (GPL-3.0)
// Extracted modules: apihashes, decr (XOR/AES/RC4/base64), lit (constants)

#include <cstdint>
#include <string>
#include <vector>
#include <optional>

namespace omnibyte::deob {

/// Platform type for API hash lookup.
enum class Platform {
    Windows,
    Linux,
    Android,
};

/// Hash entry for API resolution.
struct ApiHashEntry {
    uint64_t hash = 0;
    std::string name;
    Platform platform = Platform::Linux;
};

/// Constant analysis result.
struct ConstantInfo {
    uintptr_t offset = 0;
    uint64_t value = 0;
    int width = 0;  // bytes
    std::string context;  // description
};

/// Crypto constant detection result.
struct CryptoConstant {
    uintptr_t offset = 0;
    std::string algorithm;  // AES, DES, SHA256, etc.
    double confidence = 0.0;
};

/// hrtng-derived API hash resolution.
class HrtngApiHashes {
public:
    HrtngApiHashes();

    /// Find API name by hash for given platform.
    std::optional<std::string> findApiByHash(uint64_t hash, Platform platform) const;

    /// Register custom hash table.
    void registerHashList(Platform platform, const std::vector<ApiHashEntry>& entries);

private:
    std::vector<ApiHashEntry> entries_;
    void initBuiltinHashes();
};

/// hrtng-derived decryption routines.
class HrtngDecrypt {
public:
    /// XOR decrypt data with key.
    static std::vector<uint8_t> decryptXor(const std::vector<uint8_t>& data,
                                           const std::vector<uint8_t>& key);

    /// Base64 decode.
    static std::vector<uint8_t> decodeBase64(const std::string& encoded);

    /// Base64 encode.
    static std::string encodeBase64(const std::vector<uint8_t>& data);

    /// Simple XOR with single byte key.
    static std::vector<uint8_t> xorSingleByte(const std::vector<uint8_t>& data, uint8_t key);

    /// ROT13 decode (for string obfuscation).
    static std::vector<uint8_t> rot13(const std::vector<uint8_t>& data);

    /// Add/subtract with constant.
    static std::vector<uint8_t> addConstant(const std::vector<uint8_t>& data, int8_t delta);
};

/// hrtng-derived constant/literal analysis.
class HrtngLiterals {
public:
    /// Analyze code region for interesting constants.
    std::vector<ConstantInfo> analyzeConstants(const uint8_t* code, size_t size) const;

    /// Find potential crypto constants in data.
    std::vector<CryptoConstant> findCryptoConstants(const uint8_t* data, size_t size) const;
};

}  // namespace omnibyte::deob
