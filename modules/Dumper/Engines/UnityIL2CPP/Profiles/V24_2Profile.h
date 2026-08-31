#pragma once
// ── Profiles/V24_2Profile.h ─────────────────────────────────────
// IL2CPP metadata version 24.2 (detected when v24 stringLiteralOffset == 264).
// Source: Perfare/Il2CppDumper @ 4741d46, Il2Cpp/MetadataClass.cs
// Header: DIFFERENT from V24/V24.1 — rgctx fields REMOVED (version > 24.1)
#include "../../../DumperCore/IEngineProfile.h"
#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>

namespace omnibyte::dumper::unityil2cpp {

class V24_2Profile : public IEngineProfile {
public:
    std::string version() const override { return "24.2"; }

    uint64_t offsetOf(const std::string& key) const override {
        // V24.2 header: rgctx fields REMOVED → imagesOffset shifts to 0xA8
        // src: Il2CppDumper/Il2Cpp/MetadataClass.cs @ 4741d46
        // Detection: v24 with stringLiteralOffset == 264 (0x108)
        static const std::unordered_map<std::string, uint64_t> kOffsets = {
            {"sanity",                            0x00},
            {"version",                           0x04},
            {"stringLiteralOffset",               0x08},
            {"stringLiteralSize",                 0x0C},
            {"stringLiteralDataOffset",           0x10},
            {"stringLiteralDataSize",             0x14},
            {"stringOffset",                      0x18},
            {"stringSize",                        0x1C},
            {"eventsOffset",                      0x20},
            {"eventsSize",                        0x24},
            {"propertiesOffset",                  0x28},
            {"propertiesSize",                    0x2C},
            {"methodsOffset",                     0x30},
            {"methodsSize",                       0x34},
            {"parameterDefaultValuesOffset",      0x38},
            {"parameterDefaultValuesSize",        0x3C},
            {"fieldDefaultValuesOffset",          0x40},
            {"fieldDefaultValuesSize",            0x44},
            {"fieldAndParameterDefaultValueDataOffset", 0x48},
            {"fieldAndParameterDefaultValueDataSize",   0x4C},
            {"fieldMarshaledSizesOffset",         0x50},
            {"fieldMarshaledSizesSize",           0x54},
            {"parametersOffset",                  0x58},
            {"parametersSize",                    0x5C},
            {"fieldsOffset",                      0x60},
            {"fieldsSize",                        0x64},
            {"genericParametersOffset",           0x68},
            {"genericParametersSize",             0x6C},
            {"genericParameterConstraintsOffset", 0x70},
            {"genericParameterConstraintsSize",   0x74},
            {"genericContainersOffset",           0x78},
            {"genericContainersSize",             0x7C},
            {"nestedTypesOffset",                 0x80},
            {"nestedTypesSize",                   0x84},
            {"interfacesOffset",                  0x88},
            {"interfacesSize",                    0x8C},
            {"vtableMethodsOffset",               0x90},
            {"vtableMethodsSize",                 0x94},
            {"interfaceOffsetsOffset",            0x98},
            {"interfaceOffsetsSize",              0x9C},
            {"typeDefinitionsOffset",             0xA0},
            {"typeDefinitionsSize",               0xA4},
            // V24.2: rgctx fields REMOVED (24.2 > 24.1) → imagesOffset is now at 0xA8
            {"imagesOffset",                      0xA8},
            {"imagesSize",                        0xAC},
            {"assembliesOffset",                  0xB0},
            {"assembliesSize",                    0xB4},
            {"metadataUsageListsOffset",          0xB8},
            {"metadataUsageListsCount",           0xBC},
            {"metadataUsagePairsOffset",          0xC0},
            {"metadataUsagePairsCount",           0xC4},
            {"fieldRefsOffset",                   0xC8},
            {"fieldRefsSize",                     0xCC},
            {"referencedAssembliesOffset",        0xD0},
            {"referencedAssembliesSize",          0xD4},
            {"attributesInfoOffset",              0xD8},
            {"attributesInfoCount",               0xDC},
            {"attributeTypesOffset",              0xE0},
            {"attributeTypesCount",               0xE4},
            {"unresolvedVirtualCallParameterTypesOffset",   0xE8},
            {"unresolvedVirtualCallParameterTypesSize",     0xEC},
            {"unresolvedVirtualCallParameterRangesOffset",  0xF0},
            {"unresolvedVirtualCallParameterRangesSize",    0xF4},
            {"windowsRuntimeTypeNamesOffset",     0xF8},
            {"windowsRuntimeTypeNamesSize",       0xFC},
            {"exportedTypeDefinitionsOffset",     0x100},
            {"exportedTypeDefinitionsSize",       0x104},
        };
        auto it = kOffsets.find(key);
        return it != kOffsets.end() ? it->second : 0;
    }

    size_t structSize(const std::string& key) const override {
        // V24.2 structs: customAttributeIndex removed (Max=24), rgctx removed (Max=24.1)
        // Il2CppTypeDefinition: no customAttributeIndex, no rgctx, but has byrefTypeIndex (Max=24.5) → 0x5C
        // Il2CppMethodDefinition: no customAttributeIndex, no methodIndex/invoker/delegate/rgctx → 0x1C
        // Il2CppFieldDefinition: no customAttributeIndex, has token → 0x0C
        // Il2CppParameterDefinition: no customAttributeIndex → 0x0C
        // Il2CppPropertyDefinition: no customAttributeIndex → 0x14
        // Il2CppEventDefinition: no customAttributeIndex → 0x18
        // Il2CppImageDefinition: has customAttributeStart/Count (Min=24.1) → 0x28
        // Il2CppAssemblyDefinition: no customAttributeIndex (Max=24), has token (Min=24.1) → 0x44
        // src: Il2CppDumper/Il2Cpp/MetadataClass.cs @ 4741d46
        static const std::unordered_map<std::string, size_t> kSizes = {
            {"Il2CppTypeDefinition",  0x5C}, // no customAttributeIndex, no rgctx, has byrefTypeIndex
            {"Il2CppMethodDefinition", 0x1C}, // no customAttributeIndex/methodIndex/invoker/delegate/rgctx
            {"Il2CppFieldDefinition",  0x0C}, // no customAttributeIndex, has token
            {"Il2CppParameterDefinition", 0x0C}, // no customAttributeIndex
            {"Il2CppPropertyDefinition", 0x14}, // no customAttributeIndex
            {"Il2CppEventDefinition",   0x18}, // no customAttributeIndex
            {"Il2CppImageDefinition",   0x28}, // has customAttributeStart/Count
            {"Il2CppAssemblyDefinition", 0x44}, // has token, no customAttributeIndex
            {"Il2CppAssemblyNameDefinition", 0x34}, // has hashValueIndex (Max=24.3)
            {"Il2CppCustomAttributeTypeRange", 0x0C}, // has token
        };
        auto it = kSizes.find(key);
        return it != kSizes.end() ? it->second : 0;
    }

    bool validate(const uint8_t* headerBytes, size_t len) const override {
        // src: Il2CppDumper/Il2Cpp/Metadata.cs @ 4741d46
        // magic = 0xFAB11BAF, raw version == 24 (sub-detected as 24.2 by stringLiteralOffset == 264)
        if (len < 8) return false;
        uint32_t magic = headerBytes[0] | (headerBytes[1] << 8) |
                         (headerBytes[2] << 16) | (headerBytes[3] << 24);
        int32_t ver = headerBytes[4] | (headerBytes[5] << 8) |
                      (headerBytes[6] << 16) | ((uint32_t)headerBytes[7] << 24);
        return magic == 0xFAB11BAF && ver == 24; // raw version field is 24; sub-version detected at runtime
    }
};

} // namespace omnibyte::dumper::unityil2cpp
