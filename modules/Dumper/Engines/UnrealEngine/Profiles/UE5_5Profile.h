#pragma once
// ── Profiles/UE5_5Profile.h ───────────────────────────────────────
// UE 5.5 — same FChunkedFixedUObjectArray layout as 5.3/5.4.
//
// Note: Dumper-7 issue #330 reports some UE 5.5.4 games need manual
// GObjects override (FChunkedFixedUObjectArray with custom layout).
// The default layout still works for most games.
//
// Source: Encryqed/Dumper-7 @ main, issue #330
//         UE4SS-RE/RE-UE4SS @ main
// Coverage: UE5.5 covered; edge cases may need per-game override.
#include "../../../DumperCore/IEngineProfile.h"
#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>

namespace omnibyte::dumper::unrealengine {

class UE5_5Profile : public IEngineProfile {
public:
    std::string version() const override { return "5.5"; }

    uint64_t offsetOf(const std::string& key) const override {
        // Same default layout as UE5.3 — some 5.5.4 games need manual override
        // src: Dumper-7/ObjectArray.cpp @ main, issue #330
        static const std::unordered_map<std::string, uint64_t> kOffsets = {
            {"FChunkedObjects.ObjectsOffset",      0x00},
            {"FChunkedObjects.MaxElementsOffset",  0x10},
            {"FChunkedObjects.NumElementsOffset",  0x14},
            {"FChunkedObjects.MaxChunksOffset",    0x18},
            {"FChunkedObjects.NumChunksOffset",    0x1C},
            {"FName.Size",                         0x08},
            {"FName.NumberOffset",                 0x04},
            {"FName.ComparisonIndexOffset",        0x00},
            {"FFieldClass.NameOffset",             0x00},
            {"FFieldClass.IdOffset",               0x08},
            {"FFieldClass.CastFlagsOffset",        0x10},
            {"FFieldClass.ClassFlagsOffset",       0x18},
            {"FFieldClass.SuperClassOffset",       0x20},
            {"FField.VftOffset",                   0x00},
            {"FField.ClassOffset",                 0x08},
            {"FField.OwnerOffset",                 0x10},
            {"FField.NextOffset",                  0x20},
            {"FField.NameOffset",                  0x28},
            {"FField.FlagsOffset",                 0x30},
            {"UStruct.HasStructBaseChain",         1},
        };
        auto it = kOffsets.find(key);
        return it != kOffsets.end() ? it->second : 0;
    }

    size_t structSize(const std::string& key) const override {
        static const std::unordered_map<std::string, size_t> kSizes = {
            {"FUObjectItem",                0x10},
            {"FNameEntry",                  0x10},
        };
        auto it = kSizes.find(key);
        return it != kSizes.end() ? it->second : 0;
    }

    std::optional<AOBPattern> patternFor(const std::string& key) const override {
        if (key == "GNamesPattern") {
            return AOBPattern{
                .bytePattern = "48 8D 05 ?? ?? ?? ?? EB 27",
                .mask        = "xx?????x",
                .addressOffsetFromMatch = 3,
                .instructionLength = 7,
            };
        }
        if (key == "GObjectsPattern") {
            return AOBPattern{
                .bytePattern = "48 8B 05 ?? ?? ?? ?? 48 8B 0C C8 48 8D 04 D1",
                .mask        = "xx?????xxxxxxxxx",
                .addressOffsetFromMatch = 3,
                .instructionLength = 7,
            };
        }
        if (key == "GWorldPattern") {
            return AOBPattern{
                .bytePattern = "48 8B 05 ?? ?? ?? ?? 48 3B C8 75",
                .mask        = "xx?????xxxx",
                .addressOffsetFromMatch = 3,
                .instructionLength = 7,
            };
        }
        return std::nullopt;
    }

    bool validate(const uint8_t* headerBytes, size_t len) const override {
        (void)headerBytes;
        return len >= 8;
    }
};

} // namespace omnibyte::dumper::unrealengine
