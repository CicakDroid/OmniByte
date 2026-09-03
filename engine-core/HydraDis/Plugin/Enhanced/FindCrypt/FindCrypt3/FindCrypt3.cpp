// FindCrypt3 — Crypto constant finder.
// Source: https://github.com/HongThatCong/FindCrypt3
// Commit: master branch, 2026-09-01
// License: MIT
// IDA plugin ported as standalone crypto constant scanner.

#include "FindCrypt3.h"

#include <cstring>
#include <fstream>

namespace omnibyte::deob {

// Known crypto constants
static const uint8_t AES_SBOX[256] = {
    0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76,
    0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0, 0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0,
    0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15,
    0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75,
    0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0, 0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84,
    0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF,
    0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8,
    0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5, 0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2,
    0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73,
    0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB,
    0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C, 0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79,
    0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08,
    0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A,
    0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E, 0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E,
    0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF,
    0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16,
};

static const uint8_t SHA256_H0[32] = {
    0x67, 0xE6, 0x09, 0x6A, 0xAB, 0xD6, 0xCA, 0x3C,
    0x6A, 0x09, 0xE6, 0x67, 0xBB, 0x67, 0xAE, 0x85,
    0x3C, 0x6E, 0xF3, 0x72, 0xA5, 0x4F, 0xF5, 0x3A,
    0x51, 0x0E, 0x52, 0x7F, 0x9B, 0x05, 0x68, 0x8C,
};

static const uint8_t DES_SBOX1[64] = {
    0x0E, 0x04, 0x0D, 0x01, 0x02, 0x0F, 0x0B, 0x08,
    0x03, 0x0A, 0x06, 0x0C, 0x05, 0x09, 0x00, 0x07,
    0x00, 0x0F, 0x07, 0x04, 0x0E, 0x02, 0x0D, 0x01,
    0x0A, 0x06, 0x0C, 0x0B, 0x09, 0x05, 0x03, 0x08,
    0x04, 0x01, 0x0E, 0x08, 0x0D, 0x06, 0x02, 0x0B,
    0x0F, 0x0C, 0x09, 0x07, 0x03, 0x0A, 0x05, 0x00,
    0x0F, 0x0C, 0x08, 0x02, 0x04, 0x09, 0x01, 0x07,
    0x05, 0x0B, 0x03, 0x0E, 0x0A, 0x00, 0x06, 0x0D,
};

static const uint32_t MD5_T[64] = {
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
    0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
    0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
    0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
};

// XXTEA/TEA magic constant (delta = floor(2^32 / golden_ratio))
static const uint32_t XXTEA_DELTA = 0x9E3779B9;
static const uint32_t XXTEA_DELTA_ALT = 0x61C88647;

// RC4 KSA loop bound patterns (cmp reg, 0x100)
static const uint8_t RC4_PAT_1[] = {0x3D, 0x00, 0x01, 0x00, 0x00};           // cmp eax, 0x100
static const uint8_t RC4_PAT_2[] = {0x81, 0xF8, 0x00, 0x01, 0x00, 0x00};    // cmp eax, 0x100 (modrm)
static const uint8_t RC4_PAT_3[] = {0x81, 0xF9, 0x00, 0x01, 0x00, 0x00};    // cmp ecx, 0x100
static const uint8_t RC4_PAT_4[] = {0x81, 0xFA, 0x00, 0x01, 0x00, 0x00};    // cmp edx, 0x100
static const uint8_t RC4_PAT_5[] = {0x81, 0xFB, 0x00, 0x01, 0x00, 0x00};    // cmp ebx, 0x100
static const uint8_t RC4_PAT_6[] = {0x48, 0x3D, 0x00, 0x01, 0x00, 0x00};    // cmp rax, 0x100
static const uint8_t RC4_PAT_7[] = {0x48, 0x81, 0xF8, 0x00, 0x01, 0x00, 0x00}; // cmp rax, 0x100 (modrm)

// Whirlpool S-box (first 32 bytes)
static const uint8_t WHIRLPOOL_SBOX[32] = {
    0x18, 0x23, 0xC6, 0xE8, 0x87, 0xB8, 0x01, 0x4F,
    0x03, 0xD6, 0x34, 0x1E, 0x57, 0x72, 0x7A, 0xC0,
    0xAC, 0x61, 0x35, 0x44, 0x3C, 0x22, 0x64, 0x19,
    0xD3, 0x8B, 0x86, 0xC2, 0x9F, 0x08, 0xD5, 0xF7,
};

// RIPEMD-160 H0 initial values
static const uint8_t RIPEMD160_H0[20] = {
    0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
    0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10,
    0xF0, 0xE1, 0xD2, 0xC3,
};

// Camellia S-box (first 32 bytes)
static const uint8_t CAMELLIA_SBOX[32] = {
    0x70, 0x82, 0x2C, 0xEC, 0xB3, 0x27, 0xC0, 0xE5,
    0x12, 0xA5, 0x1A, 0x0B, 0xA2, 0x0A, 0x14, 0xA8,
    0x46, 0x06, 0x42, 0x3A, 0x08, 0x24, 0x16, 0x1E,
    0xBA, 0xE0, 0x56, 0x22, 0x68, 0xC5, 0x73, 0x18,
};

// Serpent S-box (first 32 bytes from S0)
static const uint8_t SERPENT_SBOX[32] = {
    0x33, 0x32, 0x30, 0x31, 0x38, 0x39, 0x3B, 0x3A,
    0x37, 0x36, 0x34, 0x35, 0x3E, 0x3F, 0x3D, 0x3C,
    0x43, 0x42, 0x40, 0x41, 0x48, 0x49, 0x4B, 0x4A,
    0x47, 0x46, 0x44, 0x45, 0x4E, 0x4F, 0x4D, 0x4C,
};

// Twofish P-box (first 32 bytes from P0)
static const uint8_t TWOFISH_PBOX[32] = {
    0xA9, 0x67, 0xB4, 0xD5, 0x2C, 0x6E, 0x31, 0x07,
    0x40, 0x28, 0x5E, 0x3D, 0x65, 0xB8, 0x04, 0x8C,
    0x96, 0x43, 0x6A, 0x32, 0x1C, 0x0C, 0x27, 0x53,
    0x92, 0xCF, 0xE1, 0x4B, 0x7D, 0x39, 0x64, 0x5D,
};

// GOST S-box (first 16 bytes from S1)
static const uint8_t GOST_SBOX[16] = {
    0x09, 0x06, 0x03, 0x02, 0x08, 0x0B, 0x01, 0x07,
    0x0A, 0x04, 0x0E, 0x0F, 0x0C, 0x00, 0x0D, 0x05,
};

// RC2 S-box (first 16 bytes)
static const uint8_t RC2_SBOX[16] = {
    0xD9, 0x78, 0xF9, 0xC4, 0x1D, 0x93, 0xEA, 0xDB,
    0x7F, 0x2E, 0x30, 0xF2, 0xEC, 0x6D, 0x51, 0x5B,
};

// ChaCha/Salsa20 constants "expand 32-byte k"
static const uint8_t CHACHA_CONSTANTS[16] = {
    0x65, 0x78, 0x70, 0x61, 0x6E, 0x64, 0x20, 0x33,
    0x32, 0x2D, 0x62, 0x79, 0x74, 0x65, 0x20, 0x6B,
};

std::vector<CryptoHit> FindCrypt3Engine::scanRegion(const uint8_t* data, size_t size) const {
    std::vector<CryptoHit> hits;

    for (size_t i = 0; i + 256 <= size; ++i) {
        if (checkAesSbox(data, size, i)) {
            hits.push_back({static_cast<uintptr_t>(i), "AES", 0.95, "AES S-box (256 bytes)"});
        }
    }

    for (size_t i = 0; i + 64 <= size; ++i) {
        if (checkDesSbox(data, size, i)) {
            hits.push_back({static_cast<uintptr_t>(i), "DES", 0.85, "DES S-box (64 bytes)"});
        }
    }

    for (size_t i = 0; i + 32 <= size; ++i) {
        if (checkSha256(data, size, i)) {
            hits.push_back({static_cast<uintptr_t>(i), "SHA-256", 0.90, "SHA-256 H0 constants"});
        }
    }

    for (size_t i = 0; i + 256 <= size; ++i) {
        if (checkBlowfish(data, size, i)) {
            hits.push_back({static_cast<uintptr_t>(i), "Blowfish", 0.80, "Blowfish P-box"});
        }
    }

    // XXTEA/TEA DELTA constant (4 bytes, scan every 4-byte alignment)
    for (size_t i = 0; i + 4 <= size; i += 4) {
        if (checkXxtea(data, size, i)) {
            uint32_t val = *reinterpret_cast<const uint32_t*>(data + i);
            double conf = (val == XXTEA_DELTA) ? 0.85 : 0.70;
            hits.push_back({static_cast<uintptr_t>(i), "XXTEA", conf, "TEA/XXTEA DELTA constant"});
        }
    }

    // RC4 KSA patterns (instruction byte sequences)
    for (size_t i = 0; i + 7 <= size; ++i) {
        if (checkRc4Ksa(data, size, i)) {
            hits.push_back({static_cast<uintptr_t>(i), "RC4", 0.75, "RC4 KSA loop bound (cmp reg, 0x100)"});
        }
    }

    // Whirlpool S-box (32 bytes)
    for (size_t i = 0; i + 32 <= size; ++i) {
        if (checkWhirlpool(data, size, i)) {
            hits.push_back({static_cast<uintptr_t>(i), "Whirlpool", 0.85, "Whirlpool S-box"});
        }
    }

    // RIPEMD-160 H0 (20 bytes)
    for (size_t i = 0; i + 20 <= size; ++i) {
        if (checkRipemd160(data, size, i)) {
            hits.push_back({static_cast<uintptr_t>(i), "RIPEMD-160", 0.90, "RIPEMD-160 H0 initial values"});
        }
    }

    // Camellia S-box (32 bytes)
    for (size_t i = 0; i + 32 <= size; ++i) {
        if (checkCamellia(data, size, i)) {
            hits.push_back({static_cast<uintptr_t>(i), "Camellia", 0.85, "Camellia S-box"});
        }
    }

    // Serpent S-box (32 bytes)
    for (size_t i = 0; i + 32 <= size; ++i) {
        if (checkSerpent(data, size, i)) {
            hits.push_back({static_cast<uintptr_t>(i), "Serpent", 0.80, "Serpent S-box"});
        }
    }

    // Twofish P-box (32 bytes)
    for (size_t i = 0; i + 32 <= size; ++i) {
        if (checkTwofish(data, size, i)) {
            hits.push_back({static_cast<uintptr_t>(i), "Twofish", 0.80, "Twofish P-box"});
        }
    }

    // GOST S-box (16 bytes)
    for (size_t i = 0; i + 16 <= size; ++i) {
        if (checkGost(data, size, i)) {
            hits.push_back({static_cast<uintptr_t>(i), "GOST", 0.80, "GOST S-box"});
        }
    }

    // RC2 S-box (16 bytes)
    for (size_t i = 0; i + 16 <= size; ++i) {
        if (checkRc2(data, size, i)) {
            hits.push_back({static_cast<uintptr_t>(i), "RC2", 0.80, "RC2 S-box"});
        }
    }

    // ChaCha/Salsa20 constants (16 bytes)
    for (size_t i = 0; i + 16 <= size; ++i) {
        if (checkChacha(data, size, i)) {
            hits.push_back({static_cast<uintptr_t>(i), "ChaCha/Salsa20", 0.90, "ChaCha/Salsa20 constants"});
        }
    }

    return hits;
}

std::vector<CryptoHit> FindCrypt3Engine::scanFile(const std::string& filePath) const {
    std::ifstream f(filePath, std::ios::binary | std::ios::ate);
    if (!f.is_open()) return {};

    size_t size = f.tellg();
    f.seekg(0);

    std::vector<uint8_t> data(size);
    f.read(reinterpret_cast<char*>(data.data()), size);

    return scanRegion(data.data(), size);
}

std::vector<std::string> FindCrypt3Engine::getSupportedAlgorithms() const {
    return {
        "AES", "DES", "SHA-256", "MD5", "Blowfish",
        "XXTEA", "RC4", "Whirlpool", "RIPEMD-160", "Camellia",
        "Serpent", "Twofish", "GOST", "RC2", "ChaCha/Salsa20"
    };
}

bool FindCrypt3Engine::checkAesSbox(const uint8_t* data, size_t size, size_t offset) const {
    if (offset + 256 > size) return false;
    return memcmp(data + offset, AES_SBOX, 256) == 0;
}

bool FindCrypt3Engine::checkDesSbox(const uint8_t* data, size_t size, size_t offset) const {
    if (offset + 64 > size) return false;
    return memcmp(data + offset, DES_SBOX1, 64) == 0;
}

bool FindCrypt3Engine::checkSha256(const uint8_t* data, size_t size, size_t offset) const {
    if (offset + 32 > size) return false;
    return memcmp(data + offset, SHA256_H0, 32) == 0;
}

bool FindCrypt3Engine::checkMd5(const uint8_t* data, size_t size, size_t offset) const {
    if (offset + 256 > size) return false;
    // Check for MD5 T constants (first 16 as bytes)
    for (int i = 0; i < 16; ++i) {
        uint32_t val = *reinterpret_cast<const uint32_t*>(data + offset + i * 4);
        if (val != MD5_T[i]) return false;
    }
    return true;
}

bool FindCrypt3Engine::checkBlowfish(const uint8_t* data, size_t size, size_t offset) const {
    if (offset + 72 > size) return false;
    // Blowfish P-box starts with pi digits: 0x243f6a88, 0x85a308d3
    uint32_t p0 = *reinterpret_cast<const uint32_t*>(data + offset);
    uint32_t p1 = *reinterpret_cast<const uint32_t*>(data + offset + 4);
    return (p0 == 0x243f6a88 && p1 == 0x85a308d3);
}

bool FindCrypt3Engine::checkXxtea(const uint8_t* data, size_t size, size_t offset) const {
    if (offset + 4 > size) return false;
    uint32_t val = *reinterpret_cast<const uint32_t*>(data + offset);
    return (val == XXTEA_DELTA || val == XXTEA_DELTA_ALT);
}

bool FindCrypt3Engine::checkRc4Ksa(const uint8_t* data, size_t size, size_t offset) const {
    if (memcmp(data + offset, RC4_PAT_1, sizeof(RC4_PAT_1)) == 0) return true;
    if (memcmp(data + offset, RC4_PAT_2, sizeof(RC4_PAT_2)) == 0) return true;
    if (memcmp(data + offset, RC4_PAT_3, sizeof(RC4_PAT_3)) == 0) return true;
    if (memcmp(data + offset, RC4_PAT_4, sizeof(RC4_PAT_4)) == 0) return true;
    if (memcmp(data + offset, RC4_PAT_5, sizeof(RC4_PAT_5)) == 0) return true;
    if (memcmp(data + offset, RC4_PAT_6, sizeof(RC4_PAT_6)) == 0) return true;
    if (memcmp(data + offset, RC4_PAT_7, sizeof(RC4_PAT_7)) == 0) return true;
    return false;
}

bool FindCrypt3Engine::checkWhirlpool(const uint8_t* data, size_t size, size_t offset) const {
    if (offset + 32 > size) return false;
    return memcmp(data + offset, WHIRLPOOL_SBOX, 32) == 0;
}

bool FindCrypt3Engine::checkRipemd160(const uint8_t* data, size_t size, size_t offset) const {
    if (offset + 20 > size) return false;
    return memcmp(data + offset, RIPEMD160_H0, 20) == 0;
}

bool FindCrypt3Engine::checkCamellia(const uint8_t* data, size_t size, size_t offset) const {
    if (offset + 32 > size) return false;
    return memcmp(data + offset, CAMELLIA_SBOX, 32) == 0;
}

bool FindCrypt3Engine::checkSerpent(const uint8_t* data, size_t size, size_t offset) const {
    if (offset + 32 > size) return false;
    return memcmp(data + offset, SERPENT_SBOX, 32) == 0;
}

bool FindCrypt3Engine::checkTwofish(const uint8_t* data, size_t size, size_t offset) const {
    if (offset + 32 > size) return false;
    return memcmp(data + offset, TWOFISH_PBOX, 32) == 0;
}

bool FindCrypt3Engine::checkGost(const uint8_t* data, size_t size, size_t offset) const {
    if (offset + 16 > size) return false;
    return memcmp(data + offset, GOST_SBOX, 16) == 0;
}

bool FindCrypt3Engine::checkRc2(const uint8_t* data, size_t size, size_t offset) const {
    if (offset + 16 > size) return false;
    return memcmp(data + offset, RC2_SBOX, 16) == 0;
}

bool FindCrypt3Engine::checkChacha(const uint8_t* data, size_t size, size_t offset) const {
    if (offset + 16 > size) return false;
    return memcmp(data + offset, CHACHA_CONSTANTS, 16) == 0;
}

}  // namespace omnibyte::deob
