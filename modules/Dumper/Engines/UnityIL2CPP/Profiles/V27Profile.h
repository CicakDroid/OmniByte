#pragma once
// ── Profiles/V27Profile.h ───────────────────────────────────────
// Satu file per versi metadata yang secara eksplisit didukung.
#include "../../../DumperCore/IEngineProfile.h"
#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>

namespace omnibyte::dumper::unityil2cpp {

class V27Profile : public IEngineProfile {
public:
    std::string version() const override { return "27"; }

    uint64_t offsetOf(const std::string& key) const override {
        static const std::unordered_map<std::string, uint64_t> kOffsets = {
            {"stringLiteralOffset",      0x18},
            {"typeDefinitionsOffset",    0x2C},
            {"methodsOffset",            0x34},
            // ... field lain spesifik metadata v27
        };
        auto it = kOffsets.find(key);
        return it != kOffsets.end() ? it->second : 0;
    }

    size_t structSize(const std::string& key) const override {
        if (key == "Il2CppTypeDefinition") return 0x5C;
        return 0;
    }

    bool validate(const uint8_t* headerBytes, size_t len) const override {
        // cek magic 0xAF1BB1FA + versionField == 27 di headerBytes
        (void)headerBytes;
        return len >= 8; // simplifikasi contoh
    }
};

} // namespace omnibyte::dumper::unityil2cpp