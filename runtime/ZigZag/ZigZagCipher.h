#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace omnibyte::runtime {

/**
 * ZigZag Cipher — XOR-based byte encryption for stealth payloads.
 * Used to obfuscate config strings, hook data, and communications
 * in ZigZag stealth subsystem.
 *
 * Self-inverse: encrypt(data, key) == decrypt(data, key).
 * Multi-pass with rolling key for basic obfuscation (NOT cryptographic security).
 *
 * Usage:
 *   ZigZagCipher cipher("my-secret-key");
 *   auto encrypted = cipher.encrypt(payload);
 *   auto decrypted = cipher.decrypt(encrypted);  // same as original
 */
class ZigZagCipher {
public:
    explicit ZigZagCipher(const std::string& key) : key_(key.begin(), key.end()) {}

    explicit ZigZagCipher(const uint8_t* keyData, size_t keyLen)
        : key_(keyData, keyData + keyLen) {}

    /**
     * Encrypt byte array. XOR with rolling key.
     */
    std::vector<uint8_t> encrypt(const uint8_t* data, size_t len) const {
        return process(data, len);
    }

    /**
     * Encrypt std::string payload.
     */
    std::vector<uint8_t> encrypt(const std::string& data) const {
        return process(reinterpret_cast<const uint8_t*>(data.data()), data.size());
    }

    /**
     * Decrypt byte array. Self-inverse of encrypt.
     */
    std::vector<uint8_t> decrypt(const uint8_t* data, size_t len) const {
        return process(data, len);
    }

    /**
     * Decrypt std::vector payload.
     */
    std::vector<uint8_t> decrypt(const std::vector<uint8_t>& data) const {
        return process(data.data(), data.size());
    }

    /**
     * Multi-pass encryption for stronger obfuscation.
     * Each pass XORs with shifted key offset.
     */
    std::vector<uint8_t> encryptMultiPass(const uint8_t* data, size_t len,
                                          int passes = 3) const {
        std::vector<uint8_t> result(data, data + len);
        for (int i = 0; i < passes; i++) {
            result = processWithOffset(result.data(), result.size(), i);
        }
        return result;
    }

    /**
     * Multi-pass decryption (reverse order).
     */
    std::vector<uint8_t> decryptMultiPass(const uint8_t* data, size_t len,
                                          int passes = 3) const {
        std::vector<uint8_t> result(data, data + len);
        for (int i = passes - 1; i >= 0; i--) {
            result = processWithOffset(result.data(), result.size(), i);
        }
        return result;
    }

    /**
     * Encrypt in-place (modifies input buffer).
     */
    void encryptInPlace(uint8_t* data, size_t len) const {
        processInPlace(data, len);
    }

    /**
     * Decrypt in-place (modifies input buffer).
     */
    void decryptInPlace(uint8_t* data, size_t len) const {
        processInPlace(data, len);
    }

    /**
     * Get key as hex string (for storage/logging).
     */
    std::string keyHex() const {
        std::string hex;
        hex.reserve(key_.size() * 2);
        static const char* hexChars = "0123456789abcdef";
        for (uint8_t b : key_) {
            hex += hexChars[(b >> 4) & 0x0F];
            hex += hexChars[b & 0x0F];
        }
        return hex;
    }

private:
    std::vector<uint8_t> key_;

    std::vector<uint8_t> process(const uint8_t* data, size_t len) const {
        std::vector<uint8_t> result(len);
        if (key_.empty()) {
            std::memcpy(result.data(), data, len);
            return result;
        }
        for (size_t i = 0; i < len; i++) {
            result[i] = data[i] ^ key_[i % key_.size()];
        }
        return result;
    }

    std::vector<uint8_t> processWithOffset(const uint8_t* data, size_t len,
                                           size_t keyOffset) const {
        std::vector<uint8_t> result(len);
        if (key_.empty()) {
            std::memcpy(result.data(), data, len);
            return result;
        }
        for (size_t i = 0; i < len; i++) {
            result[i] = data[i] ^ key_[(i + keyOffset) % key_.size()];
        }
        return result;
    }

    void processInPlace(uint8_t* data, size_t len) const {
        if (key_.empty()) return;
        for (size_t i = 0; i < len; i++) {
            data[i] ^= key_[i % key_.size()];
        }
    }
};

} // namespace omnibyte::runtime

#ifdef ZIGZAGCIPHER_TEST
#include <cassert>
int main() {
    using namespace omnibyte::runtime;

    ZigZagCipher cipher("test-key-12345");

    std::string original = "Hello, OmniByte!";
    auto encrypted = cipher.encrypt(original);
    auto decrypted = cipher.decrypt(encrypted);

    std::string result(decrypted.begin(), decrypted.end());
    assert(result == original);
    assert(encrypted != std::vector<uint8_t>(original.begin(), original.end()));

    auto mp = cipher.encryptMultiPass(
        reinterpret_cast<const uint8_t*>(original.data()), original.size());
    auto mpDec = cipher.decryptMultiPass(mp.data(), mp.size());
    std::string mpResult(mpDec.begin(), mpDec.end());
    assert(mpResult == original);

    printf("ZigZagCipher: self-check passed\n");
    return 0;
}
#endif
