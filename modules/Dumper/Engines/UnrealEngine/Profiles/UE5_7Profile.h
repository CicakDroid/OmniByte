#pragma once
// ── Profiles/UE5_7Profile.h ───────────────────────────────────────
// UE 5.7 — FFieldClass.CastFlags offset changes to 0x18.
//
// Key changes from 5.6:
//   - FFieldClass.CastFlags: 0x10 → 0x18 (Dumper-7 issue #472)
//   - FChunkedFixedUObjectArray: still default layout (changes in 5.8)
//
// Source: Encryqed/Dumper-7 @ main, Offsets.h (FFieldClass::CastFlags = 0x10, comment "0x18 on UE5.7")
//         Encryqed/Dumper-7 @ main, issue #472 (UE 5.7.2.0 CastFlags fix)
//         UE4SS-RE/RE-UE4SS @ main
// Coverage: UE5.7 fully covered; CastFlags offset verified via issue #472.
#include "../../../DumperCore/IEngineProfile.h"
#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>

namespace omnibyte::dumper::unrealengine {

class UE5_7Profile : public IEngineProfile {
public:
    std::string version() const override { return "5.7"; }

    uint64_t offsetOf(const std::string& key) const override {
        static const std::unordered_map<std::string, uint64_t> kOffsets = {
            // FChunkedFixedUObjectArray — same default layout as 5.3–5.6
            // src: Dumper-7/ObjectArray.cpp @ main
            {"FChunkedObjects.ObjectsOffset",      0x00},
            {"FChunkedObjects.MaxElementsOffset",  0x10},
            {"FChunkedObjects.NumElementsOffset",  0x14},
            {"FChunkedObjects.MaxChunksOffset",    0x18},
            {"FChunkedObjects.NumChunksOffset",    0x1C},
            // FName — same default config
            {"FName.Size",                         0x08},
            {"FName.NumberOffset",                 0x04},
            {"FName.ComparisonIndexOffset",        0x00},
            // FFieldClass — CastFlags moved to 0x18 in UE5.7
            // src: Dumper-7/Offsets.h @ main, comment on CastFlags
            //      Dumper-7 issue #472 (UE 5.7.2.0, CastFlags = 0x18)
            {"FFieldClass.NameOffset",             0x00},
            {"FFieldClass.IdOffset",               0x08},
            {"FFieldClass.CastFlagsOffset",        0x18},  // CHANGED from 0x10
            {"FFieldClass.ClassFlagsOffset",       0x20},  // shifted +0x8
            {"FFieldClass.SuperClassOffset",       0x28},  // shifted +0x8
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

    bool validate(const uint8_t* headerBytes, size_t len) const override {
        (void)headerBytes;
        return len >= 8;
    }
};

} // namespace omnibyte::dumper::unrealengine
