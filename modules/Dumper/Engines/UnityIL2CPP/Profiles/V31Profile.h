#pragma once
// ── Profiles/V31Profile.h ───────────────────────────────────────
// IL2CPP metadata version 31 (Unity 2022.2+).
// Source: Perfare/Il2CppDumper @ 4741d46, Il2Cpp/MetadataClass.cs
// Header: SAME layout as V29; struct differences in Il2CppMethodDefinition (adds returnParameterToken)
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
        // IDENTICAL header layout to V29
        // src: Il2CppDumper/Il2Cpp/MetadataClass.cs @ 4741d46
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
            {"imagesOffset",                      0xA8},
            {"imagesSize",                        0xAC},
            {"assembliesOffset",                  0xB0},
            {"assembliesSize",                    0xB4},
            {"fieldRefsOffset",                   0xB8},
            {"fieldRefsSize",                     0xBC},
            {"referencedAssembliesOffset",        0xC0},
            {"referencedAssembliesSize",          0xC4},
            {"attributeDataOffset",               0xC8},
            {"attributeDataSize",                 0xCC},
            {"attributeDataRangeOffset",          0xD0},
            {"attributeDataRangeSize",            0xD4},
            {"unresolvedVirtualCallParameterTypesOffset",   0xD8},
            {"unresolvedVirtualCallParameterTypesSize",     0xDC},
            {"unresolvedVirtualCallParameterRangesOffset",  0xE0},
            {"unresolvedVirtualCallParameterRangesSize",    0xE4},
            {"windowsRuntimeTypeNamesOffset",     0xE8},
            {"windowsRuntimeTypeNamesSize",       0xEC},
            {"windowsRuntimeStringsOffset",       0xF0},
            {"windowsRuntimeStringsSize",         0xF4},
            {"exportedTypeDefinitionsOffset",     0xF8},
            {"exportedTypeDefinitionsSize",       0xFC},
        };
        auto it = kOffsets.find(key);
        return it != kOffsets.end() ? it->second : 0;
    }

    size_t structSize(const std::string& key) const override {
        // V31 structs: Il2CppMethodDefinition now has returnParameterToken (Min=31)
        // Il2CppTypeDefinition: same as V27/V29 (no customAttributeIndex, no rgctx, no byrefTypeIndex)
        // src: Il2CppDumper/Il2Cpp/MetadataClass.cs @ 4741d46
        static const std::unordered_map<std::string, size_t> kSizes = {
            {"Il2CppTypeDefinition",  0x58}, // same as v27/v29
            {"Il2CppMethodDefinition", 0x20}, // CHANGED: adds returnParameterToken (Min=31) → +4 bytes
            {"Il2CppFieldDefinition",  0x0C}, // same as v27/v29
            {"Il2CppParameterDefinition", 0x0C}, // same as v27/v29
            {"Il2CppPropertyDefinition", 0x14}, // same as v27/v29
            {"Il2CppEventDefinition",   0x18}, // same as v27/v29
            {"Il2CppImageDefinition",   0x28}, // same as v27/v29
            {"Il2CppAssemblyDefinition", 0x44}, // same as v27/v29
            {"Il2CppAssemblyNameDefinition", 0x30}, // same as v27/v29
            {"Il2CppCustomAttributeDataRange", 0x08}, // same as v29
        };
        auto it = kSizes.find(key);
        return it != kSizes.end() ? it->second : 0;
    }

    bool validate(const uint8_t* headerBytes, size_t len) const override {
        // src: Il2CppDumper/Il2Cpp/Metadata.cs @ 4741d46
        if (len < 8) return false;
        uint32_t magic = headerBytes[0] | (headerBytes[1] << 8) |
                         (headerBytes[2] << 16) | (headerBytes[3] << 24);
        int32_t ver = headerBytes[4] | (headerBytes[5] << 8) |
                      (headerBytes[6] << 16) | ((uint32_t)headerBytes[7] << 24);
        return magic == 0xFAB11BAF && ver == 31;
    }
};

} // namespace omnibyte::dumper::unityil2cpp
