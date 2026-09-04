#pragma once
// DeCrypt3 — Decrypt algorithms detected by FindCrypt3.
// Reverse-engineering tool: decrypt encrypted data in binaries using
// algorithms identified via FindCrypt3 constant scanning.
//
// Sources:
//   - FIPS 197 (AES): https://csrc.nist.gov/publications/detail/fips/197/final
//   - FIPS 46-3 (DES): https://csrc.nist.gov/publications/detail/fips/46/3/final
//   - RFC 7914 (ChaCha20): https://datatracker.ietf.org/doc/html/rfc7914
//   - RFC 6229 (RC4): https://datatracker.ietf.org/doc/html/rfc6229
//   - TEA/XTEA: https://en.wikipedia.org/wiki/XTEA
//   - XXTEA: https://en.wikipedia.org/wiki/XXTEA
//   - RFC 4648 (Base64): https://datatracker.ietf.org/doc/html/rfc4648

#include <cstdint>
#include <string>
#include <vector>
#include <functional>

namespace omnibyte::deob {

/// Supported cipher algorithms for decryption.
enum class CipherAlgorithm {
    AES_ECB,
    AES_CBC,
    AES_CTR,
    DES_ECB,
    DES_CBC,
    TRIPLE_DES_ECB,
    TRIPLE_DES_CBC,
    RC4,
    RC5,
    BLOWFISH_ECB,
    BLOWFISH_CBC,
    CHACHA20,
    TEA,
    XTEA,
    XXTEA,
    XOR_SINGLE,
    XOR_MULTIPLE,
    XOR_ROLLING,
    BASE64_DECODE,
    HEX_DECODE,
    SKIPJACK,
    CAST128_ECB,
    CAST256_ECB,
};

/// Decryption result.
struct DecryptResult {
    bool success = false;
    std::string errorMessage;
    std::vector<uint8_t> plaintext;
    CipherAlgorithm algorithm;
    std::string algorithmName;
};

/// Decryption parameters.
struct DecryptParams {
    std::vector<uint8_t> ciphertext;
    std::vector<uint8_t> key;
    std::vector<uint8_t> iv;  // Initialization vector (for CBC/CTR modes)
    CipherAlgorithm algorithm;

    /// For XOR: single-byte key value
    uint8_t xorSingleKey = 0;

    /// For XOR_ROLLING: key to roll through
    std::vector<uint8_t> xorRollingKey;

    /// For XXTEA: number of rounds (0 = auto)
    uint32_t xxteaRounds = 0;
};

/// Decryption engine.
/// Implements actual cryptographic algorithms for decrypting binary data.
class DeCrypt3Engine {
public:
    DeCrypt3Engine() = default;
    ~DeCrypt3Engine() = default;

    /// Decrypt data using the specified algorithm and parameters.
    DecryptResult decrypt(const DecryptParams& params) const;

    /// Get list of supported algorithms.
    std::vector<std::string> getSupportedAlgorithms() const;

    /// Check if a specific algorithm is supported.
    bool isAlgorithmSupported(CipherAlgorithm algo) const;

    // ── Encoding utilities ───────────────────────────────────────
    /// Base64 decode.
    /// Source: RFC 4648 Section 4
    std::vector<uint8_t> base64Decode(const std::string& encoded) const;

    /// Hex string decode.
    std::vector<uint8_t> hexDecode(const std::string& hex) const;

private:
    // ── AES ──────────────────────────────────────────────────────
    /// AES-128/192/256 key expansion.
    /// Source: FIPS 197 Section 5.2
    void aesExpandKey(const uint8_t* key, int keyLen,
                      uint32_t roundKeys[60]) const;

    /// AES single block decrypt (ECB).
    /// Source: FIPS 197 Section 5.3 (Inverse Cipher)
    void aesDecryptBlock(const uint8_t* in, uint8_t* out,
                         const uint32_t roundKeys[60], int nr) const;

    /// AES CBC mode decrypt (processes multiple blocks).
    void aesCbcDecrypt(const uint8_t* in, uint8_t* out, size_t len,
                       const uint32_t roundKeys[60], int nr,
                       const uint8_t* iv) const;

    /// AES CTR mode decrypt (actually XOR with keystream).
    void aesCtrDecrypt(const uint8_t* in, uint8_t* out, size_t len,
                       const uint32_t roundKeys[60], int nr,
                       const uint8_t* iv) const;

    // ── DES / 3DES ───────────────────────────────────────────────
    /// DES key schedule.
    /// Source: FIPS 46-3 Section 7.2
    void desExpandKey(const uint8_t key[8], uint64_t subKeys[16]) const;

    /// DES single block decrypt.
    /// Source: FIPS 46-3 Section 6.2 (Decryption algorithm)
    void desDecryptBlock(const uint8_t* in, uint8_t* out,
                         const uint64_t subKeys[16]) const;

    /// 3DES key schedule (3 keys).
    void tripleDesExpandKey(const uint8_t key[24],
                            uint64_t subKeys1[16],
                            uint64_t subKeys2[16],
                            uint64_t subKeys3[16]) const;

    /// 3DES single block decrypt: D1(E2(D3(ciphertext)))
    void tripleDesDecryptBlock(const uint8_t* in, uint8_t* out,
                               const uint64_t subKeys1[16],
                               const uint64_t subKeys2[16],
                               const uint64_t subKeys3[16]) const;

    /// DES/3DES CBC mode decrypt.
    void desCbcDecrypt(const uint8_t* in, uint8_t* out, size_t len,
                       const uint8_t* iv, bool isTriple,
                       const uint64_t subKeys1[16],
                       const uint64_t subKeys2[16],
                       const uint64_t subKeys3[16]) const;

    // ── RC4 ──────────────────────────────────────────────────────
    /// RC4 KSA + PRGA decrypt (stream cipher = XOR with keystream).
    /// Source: RFC 6229
    void rc4Decrypt(const uint8_t* in, uint8_t* out, size_t len,
                    const uint8_t* key, size_t keyLen) const;

    // ── Blowfish ─────────────────────────────────────────────────
    /// Blowfish key schedule.
    /// Source: Bruce Schneier, "Applied Cryptography" Section 7.3
    void blowfishExpandKey(uint32_t P[18], uint32_t S[4][256],
                           const uint8_t* key, size_t keyLen) const;

    /// Blowfish single block decrypt.
    void blowfishDecryptBlock(const uint8_t* in, uint8_t* out,
                              const uint32_t P[18],
                              const uint32_t S[4][256]) const;

    /// Blowfish CBC mode decrypt.
    void blowfishCbcDecrypt(const uint8_t* in, uint8_t* out, size_t len,
                            const uint8_t* iv,
                            const uint32_t P[18],
                            const uint32_t S[4][256]) const;

    // ── ChaCha20 ─────────────────────────────────────────────────
    /// ChaCha20 block function.
    /// Source: RFC 7914 Section 2.1
    void chacha20Block(uint8_t out[64], const uint8_t key[32],
                       const uint8_t nonce[12], uint32_t counter) const;

    /// ChaCha20 decrypt (XOR with keystream).
    void chacha20Decrypt(const uint8_t* in, uint8_t* out, size_t len,
                         const uint8_t key[32],
                         const uint8_t nonce[12],
                         uint32_t counter) const;

    // ── TEA / XTEA / XXTEA ──────────────────────────────────────
    /// TEA decrypt a single 64-bit block.
    /// Source: Wheeler & Needham, "TEA, a Tiny Encryption Algorithm"
    void teaDecrypt(uint32_t v[2], const uint32_t k[4]) const;

    /// XTEA decrypt a single 64-bit block.
    /// Source: Needham & Wheeler, "XTEA corrections" (1998)
    void xteaDecrypt(uint32_t v[2], const uint32_t k[4]) const;

    /// XXTEA decrypt an array of uint32s.
    /// Source: Wheeler & Needham, "XXTEA corrections" (2004)
    void xxteaDecrypt(uint32_t* v, int n, const uint32_t k[4]) const;

    // ── XOR variants ─────────────────────────────────────────────
    /// Single-byte XOR decrypt.
    void xorSingleDecrypt(const uint8_t* in, uint8_t* out, size_t len,
                          uint8_t key) const;

    /// Multi-byte XOR decrypt.
    void xorMultiDecrypt(const uint8_t* in, uint8_t* out, size_t len,
                         const uint8_t* key, size_t keyLen) const;

    /// Rolling XOR decrypt (key byte increments each block).
    void xorRollingDecrypt(const uint8_t* in, uint8_t* out, size_t len,
                           const uint8_t* key, size_t keyLen) const;

    // ── Skipjack ─────────────────────────────────────────────────
    /// Skipjack single block decrypt.
    /// Source: NSA Skipjack specification (declassified 1998)
    void skipjackDecryptBlock(const uint8_t in[8], uint8_t out[8],
                              const uint8_t key[10]) const;

    // ── CAST-128 ─────────────────────────────────────────────────
    /// CAST-128 key schedule.
    /// Source: RFC 2144
    void cast128ExpandKey(const uint8_t* key, size_t keyLen,
                          uint32_t K[32]) const;

    /// CAST-128 single block decrypt.
    void cast128DecryptBlock(const uint8_t* in, uint8_t* out,
                             const uint32_t K[32]) const;

    // ── CAST-256 ─────────────────────────────────────────────────
    /// CAST-256 key schedule.
    /// Source: RFC 2612
    void cast256ExpandKey(const uint8_t* key, size_t keyLen,
                          uint32_t K[48]) const;

    /// CAST-256 single block decrypt.
    void cast256DecryptBlock(const uint8_t* in, uint8_t* out,
                             const uint32_t K[48]) const;

    // ── Helpers ──────────────────────────────────────────────────
    /// XOR two 16-byte blocks.
    void xorBlocks(uint8_t* a, const uint8_t* b, size_t len) const;

    /// Left rotate for TEA/XTEA.
    static uint32_t rotl32(uint32_t x, int n) {
        return (x << n) | (x >> (32 - n));
    }

    /// Right rotate for ChaCha20.
    static uint32_t rotr32(uint32_t x, int n) {
        return (x >> n) | (x << (32 - n));
    }
};

}  // namespace omnibyte::deob
