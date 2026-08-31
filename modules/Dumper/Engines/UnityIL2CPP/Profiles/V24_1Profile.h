#pragma once
// ── Profiles/V24_1Profile.h ─────────────────────────────────────
// IL2CPP metadata version 24.1 (detected when v24 imageDefs have token != 1).
// Source: Perfare/Il2CppDumper @ 4741d46, Il2Cpp/MetadataClass.cs
// Header: IDENTICAL layout to V24 — rgctx fields still present (version <= 24.1)
#include "../../../DumperCore/IEngineProfile.h"
#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>

namespace omnibyte::dumper::unityil2cpp {

class V24_1Profile : public IEngineProfile {
public:
    std::string version() const override { return "24.1"; }

    uint64_t offsetOf(const std::string& key) const override {
        // IDENTICAL header layout to V24 — rgctx fields still present (24.1 <= 24.1)
        // src: Il2CppDumper/Il2Cpp/MetadataClass.cs @ 4741d46
        // Detection: v24 with imageDefs[i].token != 1
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
            {"rgctxEntriesOffset",                0xA8}, // V24.1: still present (24.1 <= 24.1)
            {"rgctxEntriesCount",                 0xAC},
            {"imagesOffset",                      0xB0},
            {"imagesSize",                        0xB4},
            {"assembliesOffset",                  0xB8},
            {"assembliesSize",                    0xBC},
            {"metadataUsageListsOffset",          0xC0},
            {"metadataUsageListsCount",           0xC4},
            {"metadataUsagePairsOffset",          0xC8},
            {"metadataUsagePairsCount",           0xCC},
            {"fieldRefsOffset",                   0xD0},
            {"fieldRefsSize",                     0xD4},
            {"referencedAssembliesOffset",        0xD8},
            {"referencedAssembliesSize",          0xDC},
            {"attributesInfoOffset",              0xE0},
            {"attributesInfoCount",               0xE4},
            {"attributeTypesOffset",              0xE8},
            {"attributeTypesCount",               0xEC},
            {"unresolvedVirtualCallParameterTypesOffset",   0xF0},
            {"unresolvedVirtualCallParameterTypesSize",     0xF4},
            {"unresolvedVirtualCallParameterRangesOffset",  0xF8},
            {"unresolvedVirtualCallParameterRangesSize",    0xFC},
            {"windowsRuntimeTypeNamesOffset",     0x100},
            {"windowsRuntimeTypeNamesSize",       0x104},
            {"exportedTypeDefinitionsOffset",     0x108},
            {"exportedTypeDefinitionsSize",       0x10C},
        };
        auto it = kOffsets.find(key);
        return it != kOffsets.end() ? it->second : 0;
    }

    size_t structSize(const std::string& key) const override {
        // V24.1 structs: same as V24 for most types
        // Il2CppTypeDefinition: still has customAttributeIndex (Max=24) + rgctx (Max=24.1) + byrefTypeIndex (Max=24.5)
        // Il2CppMethodDefinition: has customAttributeIndex (Max=24) + methodIndex/invoker/delegate/rgctx (Max=24.1)
        // Il2CppImageDefinition: NOW has customAttributeStart/Count (Min=24.1) → 0x28
        // Il2CppFieldDefinition: NOW has token (Min=19), still has customAttributeIndex (Max=24) → still 0x10
        // Il2CppAssemblyDefinition: NOW has token (Min=24.1), still has customAttributeIndex (Max=24) → still 0x44
        // src: Il2CppDumper/Il2Cpp/MetadataClass.cs @ 4741d46
        static const std::unordered_map<std::string, size_t> kSizes = {
            {"Il2CppTypeDefinition",  0x70}, // same as v24 (has rgctx + byrefTypeIndex)
            {"Il2CppMethodDefinition", 0x34}, // same as v24 (has methodIndex/invoker/delegate/rgctx)
            {"Il2CppFieldDefinition",  0x10}, // same as v24 (has customAttributeIndex)
            {"Il2CppParameterDefinition", 0x10}, // same as v24
            {"Il2CppPropertyDefinition", 0x18}, // same as v24
            {"Il2CppEventDefinition",   0x1C}, // same as v24
            {"Il2CppImageDefinition",   0x28}, // CHANGED: now has customAttributeStart/Count
            {"Il2CppAssemblyDefinition", 0x44}, // same as v24 (token replaces customAttributeIndex)
            {"Il2CppAssemblyNameDefinition", 0x34}, // same as v24 (has hashValueIndex)
            {"Il2CppCustomAttributeTypeRange", 0x0C}, // CHANGED: now has token field
        };
        auto it = kSizes.find(key);
        return it != kSizes.end() ? it->second : 0;
    }

    bool validate(const uint8_t* headerBytes, size_t len) const override {
        // src: Il2CppDumper/Il2Cpp/Metadata.cs @ 4741d46
        // magic = 0xFAB11BAF, version == 24 (sub-detected as 24.1 by imageDefs token)
        if (len < 8) return false;
        uint32_t magic = headerBytes[0] | (headerBytes[1] << 8) |
                         (headerBytes[2] << 16) | (headerBytes[3] << 24);
        int32_t ver = headerBytes[4] | (headerBytes[5] << 8) |
                      (headerBytes[6] << 16) | ((uint32_t)headerBytes[7] << 24);
        return magic == 0xFAB11BAF && ver == 24; // raw version field is 24; sub-version detected at runtime
    }
};

} // namespace omnibyte::dumper::unityil2cpp
