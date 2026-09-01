// DumperCore/SharedUtils — shared utility implementations.
// File I/O, memory read, hex encoding, base64, AOB pattern scanning.
#include "SharedUtils.h"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>

namespace omnibyte::dumper::utils {

// ── File I/O ──

std::vector<uint8_t> readFileBytes(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return {};

    auto size = file.tellg();
    if (size <= 0) return {};

    std::vector<uint8_t> buffer(static_cast<size_t>(size));
    file.seekg(0);
    file.read(reinterpret_cast<char*>(buffer.data()), size);
    return buffer;
}

std::optional<std::vector<uint8_t>> readFileAt(const std::string& path,
                                                size_t offset, size_t count) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) return std::nullopt;

    file.seekg(static_cast<std::streamoff>(offset));
    if (file.fail()) return std::nullopt;

    std::vector<uint8_t> buffer(count);
    file.read(reinterpret_cast<char*>(buffer.data()),
              static_cast<std::streamsize>(count));
    auto bytesRead = file.gcount();
    if (bytesRead <= 0) return std::nullopt;

    buffer.resize(static_cast<size_t>(bytesRead));
    return buffer;
}

bool fileExists(const std::string& path) {
    std::ifstream f(path);
    return f.good();
}

// ── Process Memory ──

bool readProcessMemory(int pid, uintptr_t address, void* out, size_t size) {
    char path[64];
    std::snprintf(path, sizeof(path), "/proc/%d/mem", pid);
    std::FILE* f = std::fopen(path, "rb");
    if (!f) return false;

    if (std::fseek(f, static_cast<long>(address), SEEK_SET) != 0) {
        std::fclose(f);
        return false;
    }

    size_t read = std::fread(out, 1, size, f);
    std::fclose(f);
    return read == size;
}

std::optional<std::vector<uint8_t>> readProcessBytes(int pid, uintptr_t address,
                                                      size_t count) {
    std::vector<uint8_t> buffer(count);
    if (!readProcessMemory(pid, address, buffer.data(), count)) {
        return std::nullopt;
    }
    return buffer;
}

// ── Hex / Base64 ──

static const char kHexChars[] = "0123456789abcdef";

std::string hexEncode(const uint8_t* data, size_t len) {
    std::string result;
    result.reserve(len * 2);
    for (size_t i = 0; i < len; ++i) {
        result.push_back(kHexChars[(data[i] >> 4) & 0x0F]);
        result.push_back(kHexChars[data[i] & 0x0F]);
    }
    return result;
}

static int hexCharToNibble(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

std::optional<std::vector<uint8_t>> hexDecode(const std::string& hex) {
    if (hex.size() % 2 != 0) return std::nullopt;

    std::vector<uint8_t> result;
    result.reserve(hex.size() / 2);

    for (size_t i = 0; i < hex.size(); i += 2) {
        int hi = hexCharToNibble(hex[i]);
        int lo = hexCharToNibble(hex[i + 1]);
        if (hi < 0 || lo < 0) return std::nullopt;
        result.push_back(static_cast<uint8_t>((hi << 4) | lo));
    }
    return result;
}

static const char kBase64Chars[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string base64Encode(const uint8_t* data, size_t len) {
    std::string result;
    result.reserve(((len + 2) / 3) * 4);

    size_t i = 0;
    while (i + 2 < len) {
        uint32_t n = (static_cast<uint32_t>(data[i]) << 16) |
                     (static_cast<uint32_t>(data[i + 1]) << 8) |
                     static_cast<uint32_t>(data[i + 2]);
        result.push_back(kBase64Chars[(n >> 18) & 0x3F]);
        result.push_back(kBase64Chars[(n >> 12) & 0x3F]);
        result.push_back(kBase64Chars[(n >> 6) & 0x3F]);
        result.push_back(kBase64Chars[n & 0x3F]);
        i += 3;
    }

    if (i < len) {
        uint32_t n = static_cast<uint32_t>(data[i]) << 16;
        if (i + 1 < len) n |= static_cast<uint32_t>(data[i + 1]) << 8;

        size_t remaining = len - i;
        result.push_back(kBase64Chars[(n >> 18) & 0x3F]);
        result.push_back(kBase64Chars[(n >> 12) & 0x3F]);
        result.push_back(remaining > 1 ? kBase64Chars[(n >> 6) & 0x3F] : '=');
        result.push_back('=');
    }

    return result;
}

static int base64CharToValue(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;
}

std::optional<std::vector<uint8_t>> base64Decode(const std::string& str) {
    // Strip whitespace
    std::string cleaned;
    cleaned.reserve(str.size());
    for (char c : str) {
        if (c != ' ' && c != '\n' && c != '\r' && c != '\t') {
            cleaned.push_back(c);
        }
    }

    if (cleaned.size() % 4 != 0) return std::nullopt;

    std::vector<uint8_t> result;
    result.reserve((cleaned.size() / 4) * 3);

    for (size_t i = 0; i < cleaned.size(); i += 4) {
        int a = base64CharToValue(cleaned[i]);
        int b = base64CharToValue(cleaned[i + 1]);
        int c = cleaned[i + 2] == '=' ? 0 : base64CharToValue(cleaned[i + 2]);
        int d = cleaned[i + 3] == '=' ? 0 : base64CharToValue(cleaned[i + 3]);

        if (a < 0 || b < 0 || c < 0 || d < 0) return std::nullopt;

        uint32_t n = (static_cast<uint32_t>(a) << 18) |
                     (static_cast<uint32_t>(b) << 12) |
                     (static_cast<uint32_t>(c) << 6) |
                     static_cast<uint32_t>(d);

        result.push_back(static_cast<uint8_t>((n >> 16) & 0xFF));
        if (cleaned[i + 2] != '=') result.push_back(static_cast<uint8_t>((n >> 8) & 0xFF));
        if (cleaned[i + 3] != '=') result.push_back(static_cast<uint8_t>(n & 0xFF));
    }

    return result;
}

// ── String Utils ──

std::string trim(const std::string& s) {
    auto start = s.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    auto end = s.find_last_not_of(" \t\n\r");
    return s.substr(start, end - start + 1);
}

bool startsWith(const std::string& str, const std::string& prefix) {
    if (prefix.size() > str.size()) return false;
    return str.compare(0, prefix.size(), prefix) == 0;
}

bool endsWith(const std::string& str, const std::string& suffix) {
    if (suffix.size() > str.size()) return false;
    return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

std::optional<size_t> findCaseInsensitive(const std::string& haystack,
                                           const std::string& needle) {
    auto it = std::search(
        haystack.begin(), haystack.end(),
        needle.begin(), needle.end(),
        [](char a, char b) {
            return std::tolower(static_cast<unsigned char>(a)) ==
                   std::tolower(static_cast<unsigned char>(b));
        });
    if (it == haystack.end()) return std::nullopt;
    return static_cast<size_t>(std::distance(haystack.begin(), it));
}

// ── AOB Pattern Scanner ──

std::optional<uintptr_t> scanPattern(const uint8_t* base, size_t regionSize,
                                      const std::string& bytePattern,
                                      const std::string& mask) {
    // Parse hex pattern string into bytes
    std::vector<uint8_t> pattern;
    std::vector<bool> wildcard;

    std::istringstream iss(bytePattern);
    std::string token;
    size_t maskIdx = 0;

    while (std::getline(iss, token, ' ')) {
        if (token == "??" || token == "?") {
            pattern.push_back(0x00);
            wildcard.push_back(true);
        } else {
            auto decoded = hexDecode(token);
            if (!decoded || decoded->empty()) return std::nullopt;
            pattern.push_back((*decoded)[0]);
            wildcard.push_back(maskIdx < mask.size() && mask[maskIdx] == '?');
        }
        maskIdx++;
    }

    if (pattern.empty() || pattern.size() > regionSize) return std::nullopt;

    // Simple byte-by-byte scan
    size_t scanEnd = regionSize - pattern.size();
    for (size_t i = 0; i <= scanEnd; ++i) {
        bool found = true;
        for (size_t j = 0; j < pattern.size(); ++j) {
            if (!wildcard[j] && base[i + j] != pattern[j]) {
                found = false;
                break;
            }
        }
        if (found) {
            return reinterpret_cast<uintptr_t>(base + i);
        }
    }

    return std::nullopt;
}

std::optional<uintptr_t> scanAndExtractRIP(const uint8_t* base, size_t regionSize,
                                             const std::string& bytePattern,
                                             const std::string& mask,
                                             int addressOffset, int instructionLen) {
    auto matchAddr = scanPattern(base, regionSize, bytePattern, mask);
    if (!matchAddr) return std::nullopt;

    // Read 32-bit RIP-relative displacement at matchAddr + addressOffset
    auto disp32Bytes = readProcessBytes(0, matchAddr.value() + addressOffset, 4);
    if (!disp32Bytes) {
        // Fallback: read directly from base (if we're scanning in-process memory)
        if (static_cast<size_t>(addressOffset + 4) > regionSize) return std::nullopt;
        uint32_t disp32;
        std::memcpy(&disp32, base + (matchAddr.value() - reinterpret_cast<uintptr_t>(base)) + addressOffset, 4);
        return matchAddr.value() + addressOffset + instructionLen + static_cast<uintptr_t>(disp32);
    }

    uint32_t disp32;
    std::memcpy(&disp32, disp32Bytes->data(), 4);

    // Final address: matchAddr + addressOffset + instructionLen + disp32
    return matchAddr.value() + addressOffset + instructionLen + static_cast<uintptr_t>(disp32);
}

// ── Memory Utilities ──

bool isPageAligned(uintptr_t addr, size_t pageSize) {
    return (addr & (pageSize - 1)) == 0;
}

uintptr_t pageAlignUp(uintptr_t addr, size_t pageSize) {
    return (addr + pageSize - 1) & ~(pageSize - 1);
}

} // namespace omnibyte::dumper::utils
