#pragma once
// UnityIL2CPP — static analyzer for global-metadata.dat.
// Parses Il2CppTypeDefinition, Il2CppMethodDefinition, Il2CppFieldDefinition
// using profile offsets. Does not require live process.
#include "../../../DumperCore/IDumperEngine.h"
#include "../../../DumperCore/IEngineProfile.h"
#include "../../../DumperCore/SharedUtils/SharedUtils.h"
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

namespace omnibyte::dumper::unityil2cpp {

class UnityIL2CPPAnalyzer {
public:
    static DumpResult analyze(const AnalysisTarget& target,
                              const std::shared_ptr<IEngineProfile>& profile) {
        DumpResult result;
        result.engineName = "Unity IL2CPP";
        result.detectedVersion = profile ? profile->version() : "unknown";

        if (!profile) {
            result.errorMessage = "No profile provided";
            return result;
        }

        if (!target.isFile()) {
            result.errorMessage = "IL2CPP analyzer only supports file targets (global-metadata.dat)";
            return result;
        }

        auto fileData = utils::readFileBytes(target.filePath);
        if (fileData.empty()) {
            result.errorMessage = "Failed to read file: " + target.filePath;
            return result;
        }

        // Validate IL2CPP metadata magic
        if (fileData.size() < 16) {
            result.errorMessage = "File too small to be IL2CPP metadata";
            return result;
        }

        uint32_t magic = readU32(fileData, 0);
        if (magic != kIl2CppMagic) {
            result.errorMessage = "Invalid IL2CPP metadata magic (expected 0xAF1BB1FA)";
            return result;
        }

        uint32_t version = readU32(fileData, 4);
        result.setMeta("metadataVersion", std::to_string(version));

        // Use profile offsets to locate metadata sections
        return parseMetadata(fileData, profile, result);
    }

private:
    static const uint32_t kIl2CppMagic = 0xAF1BB1FA;

    // Helper read functions
    static uint32_t readU32(const std::vector<uint8_t>& buf, size_t off) {
        if (off + 4 > buf.size()) return 0;
        uint32_t v;
        std::memcpy(&v, buf.data() + off, 4);
        return v;
    }

    static uint64_t readU64(const std::vector<uint8_t>& buf, size_t off) {
        if (off + 8 > buf.size()) return 0;
        uint64_t v;
        std::memcpy(&v, buf.data() + off, 8);
        return v;
    }

    // Read string from string heap (IL2CPP string literals are stored as
    // offset into a data section)
    static std::string readStringFromHeap(const std::vector<uint8_t>& data,
                                           size_t heapOffset, uint32_t strOffset) {
        size_t absOffset = heapOffset + strOffset;
        if (absOffset >= data.size()) return "";

        // Find null terminator
        size_t maxLen = data.size() - absOffset;
        size_t len = strnlen(reinterpret_cast<const char*>(data.data() + absOffset), maxLen);
        return std::string(reinterpret_cast<const char*>(data.data() + absOffset), len);
    }

    // Parse the full IL2CPP metadata
    static DumpResult parseMetadata(const std::vector<uint8_t>& data,
                                     const std::shared_ptr<IEngineProfile>& profile,
                                     DumpResult& result) {
        // IL2CPP metadata layout (version-dependent, offsets from profile):
        //   stringLiteralOffset, stringLiteralDataOffset
        //   typeDefinitionsOffset, typeDefinitionCount
        //   methodDefinitionOffset, methodDefinitionCount
        //   fieldDefinitionOffset, fieldDefinitionCount
        //   imageDefinitionOffset, imageDefinitionCount

        size_t typeDefOffset = static_cast<size_t>(
            profile->offsetOf("typeDefinitionsOffset"));
        uint32_t typeDefCount = static_cast<uint32_t>(
            profile->offsetOf("typeDefinitionCount"));
        size_t methodDefOffset = static_cast<size_t>(
            profile->offsetOf("methodDefinitionOffset"));
        uint32_t methodDefCount = static_cast<uint32_t>(
            profile->offsetOf("methodDefinitionCount"));
        size_t fieldDefOffset = static_cast<size_t>(
            profile->offsetOf("fieldDefinitionOffset"));
        uint32_t fieldDefCount = static_cast<uint32_t>(
            profile->offsetOf("fieldDefinitionCount"));
        size_t imageDefOffset = static_cast<size_t>(
            profile->offsetOf("imageDefinitionOffset"));
        uint32_t imageDefCount = static_cast<uint32_t>(
            profile->offsetOf("imageDefinitionCount"));
        size_t stringLiteralOffset = static_cast<size_t>(
            profile->offsetOf("stringLiteralOffset"));
        size_t stringLiteralDataOffset = static_cast<size_t>(
            profile->offsetOf("stringLiteralDataOffset"));

        // Get struct sizes from profile
        size_t typeDefStructSize = profile->structSize("Il2CppTypeDefinition");
        size_t methodDefStructSize = profile->structSize("Il2CppMethodDefinition");
        size_t fieldDefStructSize = profile->structSize("Il2CppFieldDefinition");
        size_t imageDefStructSize = profile->structSize("Il2CppImageDefinition");

        // Parse type definitions
        if (typeDefOffset > 0 && typeDefCount > 0 && typeDefStructSize > 0) {
            parseTypeDefinitions(data, typeDefOffset, typeDefCount,
                                 typeDefStructSize, result);
        }

        // Parse method definitions
        if (methodDefOffset > 0 && methodDefCount > 0 && methodDefStructSize > 0) {
            parseMethodDefinitions(data, methodDefOffset, methodDefCount,
                                    methodDefStructSize, result);
        }

        // Parse field definitions
        if (fieldDefOffset > 0 && fieldDefCount > 0 && fieldDefStructSize > 0) {
            parseFieldDefinitions(data, fieldDefOffset, fieldDefCount,
                                   fieldDefStructSize, result);
        }

        // Parse string literals
        if (stringLiteralOffset > 0 && stringLiteralDataOffset > 0) {
            parseStringLiterals(data, stringLiteralOffset, stringLiteralDataOffset,
                                 result);
        }

        result.setMeta("typeCount", std::to_string(result.typeTable.size()));
        result.setMeta("methodCount", std::to_string(result.methodTable.size()));
        result.setMeta("fieldCount", std::to_string(result.fieldTable.size()));
        result.setMeta("stringCount", std::to_string(result.stringTable.size()));
        result.success = true;
        return result;
    }

    // Parse Il2CppTypeDefinition array
    static void parseTypeDefinitions(const std::vector<uint8_t>& data,
                                      size_t offset, uint32_t count,
                                      size_t structSize, DumpResult& result) {
        // Il2CppTypeDefinition fields (common across versions):
        //   +0x00: nameIndex (u32) — index into string heap
        //   +0x04: namespaceIndex (u32)
        //   +0x08: bitfield (u32) — flags, generic params, etc.
        //   +0x0C: genericContainerIndex (u32)
        //   +0x10: parentIndex (u32) — index to parent type
        //   +0x14: declaringTypeIndex (u32)
        //   +0x18: interfacesStart (u32)
        //   +0x1C: interfacesCount (u16)
        //   +0x1E: methodStart (u32)
        //   +0x22: methodCount (u16)
        //   +0x24: fieldStart (u32)
        //   +0x28: fieldCount (u16)

        for (uint32_t i = 0; i < count; ++i) {
            size_t entryOff = offset + (i * structSize);
            if (entryOff + structSize > data.size()) break;

            TypeEntry type;
            type.typeId = i;
            type.name = "Type_" + std::to_string(i);  // placeholder, resolved via string heap
            type.address = entryOff;

            // Read method count for size metadata
            if (structSize >= 0x24) {
                uint16_t methodCount = static_cast<uint16_t>(
                    readU32(data, entryOff + 0x22) & 0xFFFF);
                type.size = methodCount;  // store method count temporarily
            }

            result.typeTable.push_back(type);
        }
    }

    // Parse Il2CppMethodDefinition array
    static void parseMethodDefinitions(const std::vector<uint8_t>& data,
                                        size_t offset, uint32_t count,
                                        size_t structSize, DumpResult& result) {
        for (uint32_t i = 0; i < count; ++i) {
            size_t entryOff = offset + (i * structSize);
            if (entryOff + structSize > data.size()) break;

            MethodEntry method;
            method.methodIndex = i;
            method.name = "Method_" + std::to_string(i);

            // +0x04: declaringTypeIndex
            uint32_t declaringTypeIndex = readU32(data, entryOff + 0x04);
            method.declaringType = "Type_" + std::to_string(declaringTypeIndex);

            result.methodTable.push_back(method);
        }
    }

    // Parse Il2CppFieldDefinition array
    static void parseFieldDefinitions(const std::vector<uint8_t>& data,
                                       size_t offset, uint32_t count,
                                       size_t structSize, DumpResult& result) {
        for (uint32_t i = 0; i < count; ++i) {
            size_t entryOff = offset + (i * structSize);
            if (entryOff + structSize > data.size()) break;

            FieldEntry field;
            field.name = "Field_" + std::to_string(i);

            // +0x08: parentIndex (declaring type)
            uint32_t parentIndex = readU32(data, entryOff + 0x08);
            field.declaringType = "Type_" + std::to_string(parentIndex);

            result.fieldTable.push_back(field);
        }
    }

    // Parse string literal table
    static void parseStringLiterals(const std::vector<uint8_t>& data,
                                     size_t tableOffset, size_t dataOffset,
                                     DumpResult& result) {
        // String literal table: count (u32) + entries (length u32 + data offset u32)
        if (tableOffset + 4 > data.size()) return;

        uint32_t count = readU32(data, tableOffset);
        size_t pos = tableOffset + 4;

        for (uint32_t i = 0; i < count && pos + 8 <= data.size(); ++i) {
            uint32_t length = readU32(data, pos);
            uint32_t dataIdx = readU32(data, pos + 4);
            pos += 8;

            StringEntry entry;
            entry.address = dataOffset + dataIdx;

            // Read string from data section
            size_t strAbsOffset = dataOffset + dataIdx;
            if (strAbsOffset + length <= data.size()) {
                entry.value = std::string(
                    reinterpret_cast<const char*>(data.data() + strAbsOffset), length);
            }

            result.stringTable.push_back(entry);
        }
    }
};

// Static magic constant
const uint32_t UnityIL2CPPAnalyzer::kIl2CppMagic;

} // namespace omnibyte::dumper::unityil2cpp
