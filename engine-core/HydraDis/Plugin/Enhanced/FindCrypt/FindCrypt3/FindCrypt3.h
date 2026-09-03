#pragma once
// FindCrypt3 — Crypto constant finder.
// Source: https://github.com/HongThatCong/FindCrypt3 (MIT)

#include <cstdint>
#include <string>
#include <vector>

namespace omnibyte::deob {

/// Crypto algorithm identification result.
struct CryptoHit {
    uintptr_t offset = 0;
    std::string algorithm;
    double confidence = 0.0;
    std::string description;
};

/// Crypto constant finder engine.
/// Scans binary regions for known crypto algorithm signatures.
class FindCrypt3Engine {
public:
    FindCrypt3Engine() = default;
    ~FindCrypt3Engine() = default;

    /// Scan a memory region for crypto constants.
    std::vector<CryptoHit> scanRegion(const uint8_t* data, size_t size) const;

    /// Scan from file path.
    std::vector<CryptoHit> scanFile(const std::string& filePath) const;

    /// Get list of supported algorithms.
    std::vector<std::string> getSupportedAlgorithms() const;

private:
    /// Check for AES S-box at offset.
    bool checkAesSbox(const uint8_t* data, size_t size, size_t offset) const;

    /// Check for DES S-boxes at offset.
    bool checkDesSbox(const uint8_t* data, size_t size, size_t offset) const;

    /// Check for SHA-256 constants at offset.
    bool checkSha256(const uint8_t* data, size_t size, size_t offset) const;

    /// Check for MD5 constants at offset.
    bool checkMd5(const uint8_t* data, size_t size, size_t offset) const;

    /// Check for Blowfish S-box at offset.
    bool checkBlowfish(const uint8_t* data, size_t size, size_t offset) const;

    /// Check for XXTEA/TEA DELTA constant at offset.
    bool checkXxtea(const uint8_t* data, size_t size, size_t offset) const;

    /// Check for RC4 KSA pattern (cmp reg, 0x100) at offset.
    bool checkRc4Ksa(const uint8_t* data, size_t size, size_t offset) const;

    /// Check for Whirlpool S-box at offset.
    bool checkWhirlpool(const uint8_t* data, size_t size, size_t offset) const;

    /// Check for RIPEMD-160 H0 constants at offset.
    bool checkRipemd160(const uint8_t* data, size_t size, size_t offset) const;

    /// Check for Camellia S-box at offset.
    bool checkCamellia(const uint8_t* data, size_t size, size_t offset) const;

    /// Check for Serpent S-box at offset.
    bool checkSerpent(const uint8_t* data, size_t size, size_t offset) const;

    /// Check for Twofish P-box at offset.
    bool checkTwofish(const uint8_t* data, size_t size, size_t offset) const;

    /// Check for GOST S-box at offset.
    bool checkGost(const uint8_t* data, size_t size, size_t offset) const;

    /// Check for RC2 S-box at offset.
    bool checkRc2(const uint8_t* data, size_t size, size_t offset) const;

    /// Check for ChaCha/Salsa20 constants at offset.
    bool checkChacha(const uint8_t* data, size_t size, size_t offset) const;

    /// Check for SHA-1 H0 at offset.
    bool checkSha1(const uint8_t* data, size_t size, size_t offset) const;

    /// Check for SHA-384 H0 at offset.
    bool checkSha384(const uint8_t* data, size_t size, size_t offset) const;

    /// Check for SHA-512 H0 at offset.
    bool checkSha512(const uint8_t* data, size_t size, size_t offset) const;

    /// Check for MD2 PI_SUBST at offset.
    bool checkMd2(const uint8_t* data, size_t size, size_t offset) const;

    /// Check for MD4 T constants at offset.
    bool checkMd4(const uint8_t* data, size_t size, size_t offset) const;

    /// Check for SEED KC constants at offset.
    bool checkSeed(const uint8_t* data, size_t size, size_t offset) const;

    /// Check for LEA Delta constant at offset.
    bool checkLea(const uint8_t* data, size_t size, size_t offset) const;

    /// Check for Serpent full S-box at offset.
    bool checkSerpentFull(const uint8_t* data, size_t size, size_t offset) const;

    /// Check for Tiger IV at offset.
    bool checkTiger(const uint8_t* data, size_t size, size_t offset) const;

    /// Check for HAVAL IV at offset.
    bool checkHaval(const uint8_t* data, size_t size, size_t offset) const;

    /// Check for BLAKE2s IV at offset.
    bool checkBlake2s(const uint8_t* data, size_t size, size_t offset) const;

    /// Check for Keccak round constants at offset.
    bool checkKeccak(const uint8_t* data, size_t size, size_t offset) const;

    /// Check for SIMON-64/128 z3 constant at offset.
    bool checkSimon(const uint8_t* data, size_t size, size_t offset) const;

    /// Check for Skipjack F-table at offset.
    bool checkSkipjack(const uint8_t* data, size_t size, size_t offset) const;

    /// Check for Square S-box at offset.
    bool checkSquare(const uint8_t* data, size_t size, size_t offset) const;

    /// Check for SHARK S-box at offset.
    bool checkShark(const uint8_t* data, size_t size, size_t offset) const;

    /// Check for Curve25519 field prime at offset.
    bool checkDonna(const uint8_t* data, size_t size, size_t offset) const;

    bool checkMd5mac(const uint8_t* data, size_t size, size_t offset) const;
    bool checkShacal2(const uint8_t* data, size_t size, size_t offset) const;
    bool checkWake(const uint8_t* data, size_t size, size_t offset) const;
    bool checkModm(const uint8_t* data, size_t size, size_t offset) const;
};

}  // namespace omnibyte::deob
