#pragma once
// ── Profiles/V31Profile.h ───────────────────────────────────────
// Versi lain, offset berbeda -- TIDAK menyentuh V27Profile atau Analyzer/Resolver.
#include "../../../DumperCore/IEngineProfile.h"
#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>

namespace omnibyte::dumper::unityil2cpp {

class V31Profile : public IEngineProfile {
public:
    std::string version() const override { return "31"; }

    uint64_t offsetOf(const std::string& key) const override {
        static const std::unordered_map<std::string, uint64_t> kOffsets = {
            {"stringLiteralOffset",      0x20},   // shifted vs v27
            {"typeDefinitionsOffset",    0x30},
            {"methodsOffset",            0x38},
        };
        auto it = kOffsets.find(key);
        return it != kOffsets.end() ? it->second : 0;
    }

    size_t structSize(const std::string& key) const override {
        if (key == "Il2CppTypeDefinition") return 0x64; // beda dari v27
        return 0;
    }

    bool validate(const uint8_t* headerBytes, size_t len) const override {
        (void)headerBytes;
        return len >= 8;
    }
};

} // namespace omnibyte::dumper::unityil2cpp