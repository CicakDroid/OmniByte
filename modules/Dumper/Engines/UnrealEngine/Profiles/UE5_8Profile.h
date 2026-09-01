#pragma once
// ── Profiles/UE5_8Profile.h ───────────────────────────────────────
// UE 5.8 — FChunkedFixedUObjectArray layout CHANGED from default.
//
// Key changes from 5.7:
//   - FChunkedFixedUObjectArray: new layout (ObjectsOffset=0x00, MaxElements=0x0C,
//     NumElements=0x08, MaxChunks=0x14, NumChunks=0x10)
//   - FFieldClass.CastFlags: still 0x18 (same as 5.7)
//
// WARNING: UE5.8 data comes from development build only (Dumper-7 main branch).
//          Retail/release layout may differ. Marked as development-build validated.
//
// Source: Encryqed/Dumper-7 @ main, ObjectArray.cpp (FChunkedFixedUObjectArrayLayouts[1])
//         UE4SS-RE/RE-UE4SS @ main (partial coverage, dev build only)
// Coverage: UE5.8 — DEVELOPMENT BUILD ONLY. Retail layout may differ.
#include "../../../DumperCore/IEngineProfile.h"
#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>

namespace omnibyte::dumper::unrealengine {

class UE5_8Profile : public IEngineProfile {
public:
    std::string version() const override { return "5.8"; }

    uint64_t offsetOf(const std::string& key) const override {
        // FChunkedFixedUObjectArray — UE5.8 DEVELOPMENT BUILD layout
        // src: Dumper-7/ObjectArray.cpp @ main, FChunkedFixedUObjectArrayLayouts[1]
        static const std::unordered_map<std::string, uint64_t> kOffsets = {
            {"FChunkedObjects.ObjectsOffset",      0x00},
            {"FChunkedObjects.MaxElementsOffset",  0x0C},  // CHANGED from 0x10
            {"FChunkedObjects.NumElementsOffset",  0x08},  // CHANGED from 0x14
            {"FChunkedObjects.MaxChunksOffset",    0x14},  // CHANGED from 0x18
            {"FChunkedObjects.NumChunksOffset",    0x10},  // CHANGED from 0x1C
            // FName — same default config
            {"FName.Size",                         0x08},
            {"FName.NumberOffset",                 0x04},
            {"FName.ComparisonIndexOffset",        0x00},
            // FFieldClass — same as 5.7
            {"FFieldClass.NameOffset",             0x00},
            {"FFieldClass.IdOffset",               0x08},
            {"FFieldClass.CastFlagsOffset",        0x18},
            {"FFieldClass.ClassFlagsOffset",       0x20},
            {"FFieldClass.SuperClassOffset",       0x28},
            // FField — same layout
            {"FField.VftOffset",                   0x00},
            {"FField.ClassOffset",                 0x08},
            {"FField.OwnerOffset",                 0x10},
            {"FField.NextOffset",                  0x20},
            {"FField.NameOffset",                  0x28},
            {"FField.FlagsOffset",                 0x30},
            {"UStruct.HasStructBaseChain",         1},
            {"UEnum.IsNewNamesContainer",          1},
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

    bool validate(const uint8_t* headerBytes, size_t len) const override { return len >= 8; }
};

} // namespace omnibyte::dumper::unrealengine
