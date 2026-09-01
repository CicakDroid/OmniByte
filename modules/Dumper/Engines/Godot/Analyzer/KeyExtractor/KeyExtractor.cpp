// KeyDot — Godot encryption key extractor.
// Source: https://github.com/Titoot/KeyDot
// Commit: main branch, 2026-08-18
// License: MIT
// Static analysis of Godot binary to find encryption key patterns.

#include "KeyExtractor.h"

#include <algorithm>
#include <cstring>
#include <fstream>

namespace omnibyte::dumper {

// Godot key structure patterns (APK encryption key markers)
const std::vector<std::vector<uint8_t>> GodotKeyExtractor::keyPatterns_ = {
    // Godot 3.x encryption key marker
    {0x47, 0x4F, 0x44, 0x4F, 0x54, 0x4B, 0x45, 0x59},  // "GODOTKEY"
    // Godot 4.x key structure
    {0x47, 0x6F, 0x64, 0x6F, 0x74, 0x20, 0x4B, 0x65, 0x79},  // "Godot Key"
    // PCK header (Godot package)
    {0x47, 0x44, 0x50, 0x43, 0x4B},  // "GDPCK"
};

std::vector<ExtractedKey> GodotKeyExtractor::extractKeys(const std::string& filePath) const {
    std::ifstream f(filePath, std::ios::binary | std::ios::ate);
    if (!f.is_open()) return {};

    size_t size = f.tellg();
    f.seekg(0);

    std::vector<uint8_t> data(size);
    f.read(reinterpret_cast<char*>(data.data()), size);

    return scanForKeyPatterns(data.data(), size);
}

std::vector<ExtractedKey> GodotKeyExtractor::scanForKeyPatterns(const uint8_t* data,
                                                                 size_t size) const {
    std::vector<ExtractedKey> keys;

    for (size_t i = 0; i + 32 <= size; ++i) {
        for (size_t p = 0; p < keyPatterns_.size(); ++p) {
            const auto& pattern = keyPatterns_[p];
            if (i + pattern.size() > size) continue;

            if (memcmp(data + i, pattern.data(), pattern.size()) == 0) {
                // Found pattern — extract potential key
                // Read 32 bytes after pattern as potential key
                size_t keyLen = std::min(size_t(32), size - i - pattern.size());
                std::vector<uint8_t> keyData(data + i + pattern.size(),
                                              data + i + pattern.size() + keyLen);

                if (validateKey(keyData)) {
                    ExtractedKey key;
                    key.name = "godot_key_" + std::to_string(keys.size());
                    key.value = keyData;
                    key.type = "encryption";
                    key.offset = i;
                    key.confidence = 0.7 + (p == 0 ? 0.2 : 0.0);
                    keys.push_back(key);
                }
            }
        }
    }

    return keys;
}

bool GodotKeyExtractor::validateKey(const std::vector<uint8_t>& key) const {
    if (key.empty() || key.size() < 16) return false;

    // Check for reasonable entropy (not all zeros or repeating)
    std::vector<int> freq(256, 0);
    for (uint8_t b : key) freq[b]++;

    int unique = 0;
    for (int f : freq) {
        if (f > 0) ++unique;
    }

    // At least 8 unique byte values suggests real key material
    return unique >= 8;
}

}  // namespace omnibyte::dumper
