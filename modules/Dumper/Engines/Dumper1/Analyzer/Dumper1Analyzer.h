#pragma once
// Dumper1 — IL2CPP metadata parser (adapted from il2cpp-dumper-rs).
// Parses global-metadata.dat with XOR decryption, extracts TypeDefs, MethodDefs,
// FieldDefs, and string literals. Resolves names via string data blob lookup.
#include "../../../DumperCore/IDumperEngine.h"
#include "../../../DumperCore/IEngineProfile.h"
#include "../../../DumperCore/SharedUtils/SharedUtils.h"
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <vector>
#include <functional>

namespace omnibyte::dumper::dumper1 {

class Dumper1Analyzer {
public:
    using ProgressCallback = std::function<void(const std::string&)>;

    static DumpData analyze(const AnalysisTarget& target,
                              const std::shared_ptr<IEngineProfile>& profile,
                              ProgressCallback progress = nullptr) {
        DumpData result;
        result.engineName = "Dumper1";
        result.detectedVersion = profile ? profile->version() : "unknown";

        if (!profile) {
            result.errorMessage = "No profile provided";
            return result;
        }

        if (!target.isFile()) {
            result.errorMessage = "Dumper1 analyzer only supports file targets";
            return result;
        }

        auto fileData = utils::readFileBytes(target.filePath);
        if (fileData.empty()) {
            result.errorMessage = "Failed to read file: " + target.filePath;
            return result;
        }

        if (fileData.size() < 64) {
            result.errorMessage = "File too small to be IL2CPP metadata";
            return result;
        }

        if (progress) progress("Validating metadata magic...");

        uint32_t magic = readU32(fileData, 0);
        if (magic != kIl2CppMagic) {
            if (!tryDecryptMetadata(fileData)) {
                result.errorMessage = "Invalid IL2CPP metadata magic (expected 0xFAB11BAF). "
                                      "File may be encrypted.";
                return result;
            }
            result.setMeta("decrypted", "true");
        }

        uint32_t version = readU32(fileData, 4);
        result.setMeta("metadataVersion", std::to_string(version));

        if (progress) progress("Parsing metadata structures...");

        parseMetadata(fileData, profile, result, progress);

        result.setMeta("typeCount", std::to_string(result.typeTable.size()));
        result.setMeta("methodCount", std::to_string(result.methodTable.size()));
        result.setMeta("fieldCount", std::to_string(result.fieldTable.size()));
        result.setMeta("stringCount", std::to_string(result.stringTable.size()));
        result.success = true;
        return result;
    }

private:
    static const uint32_t kIl2CppMagic = 0xFAB11BAF;

    static uint32_t readU32(const std::vector<uint8_t>& buf, size_t off) {
        if (off + 4 > buf.size()) return 0;
        uint32_t v;
        std::memcpy(&v, buf.data() + off, 4);
        return v;
    }

    // ── Read a length-prefixed UTF-8 string from the data blob ──
    // IL2CPP string data format: 4-byte LE length, then that many bytes.
    // nameIndex is a byte offset into the string data region.
    static std::string readNameFromData(const std::vector<uint8_t>& data,
                                         size_t dataRegionStart, uint32_t nameIndex) {
        size_t pos = dataRegionStart + static_cast<size_t>(nameIndex);
        if (pos + 4 > data.size()) return "";
        uint32_t len = readU32(data, pos);
        pos += 4;
        if (pos + len > data.size() || len > 1024 * 1024) return ""; // sanity check: 1MB max
        return std::string(reinterpret_cast<const char*>(data.data() + pos), len);
    }

    // ── XOR Decryption (from il2cpp-dumper-rs try_decrypt_metadata) ──
    static bool tryDecryptMetadata(std::vector<uint8_t>& data) {
        if (data.size() < 16) return false;

        const uint8_t target[] = { 0xAF, 0x1B, 0xB1, 0xFA };

        // Single-byte XOR
        {
            uint8_t k1 = target[0] ^ data[0];
            if (k1 != 0) {
                std::vector<uint8_t> test(data.begin(), data.end());
                for (auto& b : test) b ^= k1;
                if (isValidMetadataVersion(test)) {
                    for (auto& b : data) b ^= k1;
                    return true;
                }
            }
        }

        // 4-byte XOR
        {
            uint8_t key4[4];
            bool allZero = true;
            for (int i = 0; i < 4; ++i) {
                key4[i] = target[i] ^ data[i];
                if (key4[i] != 0) allZero = false;
            }
            if (!allZero) {
                std::vector<uint8_t> test(data.begin(), data.end());
                for (size_t i = 0; i < test.size(); ++i) test[i] ^= key4[i % 4];
                if (isValidMetadataVersion(test)) {
                    for (size_t i = 0; i < data.size(); ++i) data[i] ^= key4[i % 4];
                    return true;
                }
            }
        }

        // 8-byte XOR
        {
            uint8_t key8[8];
            for (int i = 0; i < 4; ++i) key8[i] = target[i] ^ data[i];
            for (int i = 4; i < 8; ++i) key8[i] = data[i];
            if (key8[0] != 0 || key8[1] != 0 || key8[2] != 0 || key8[3] != 0) {
                std::vector<uint8_t> test(data.begin(), data.end());
                for (size_t i = 0; i < test.size(); ++i) test[i] ^= key8[i % 8];
                if (isValidMetadataVersion(test)) {
                    for (size_t i = 0; i < data.size(); ++i) data[i] ^= key8[i % 8];
                    return true;
                }
            }
        }

        // Rolling XOR (key lengths: 16, 32, 64, 128, 256)
        for (size_t keyLen : {16u, 32u, 64u, 128u, 256u}) {
            if (data.size() < keyLen * 2) continue;
            std::vector<uint8_t> key(keyLen);
            for (size_t i = 0; i < keyLen; ++i) {
                key[i] = (i < 4) ? (target[i] ^ data[i]) : data[i];
            }
            if (key[0] == 0 && key[1] == 0 && key[2] == 0 && key[3] == 0) continue;
            std::vector<uint8_t> test(data.begin(), data.begin() + 8);
            for (size_t i = 0; i < test.size(); ++i) test[i] ^= key[i % keyLen];
            if (test[0] == target[0] && test[1] == target[1] &&
                test[2] == target[2] && test[3] == target[3] &&
                isValidMetadataVersion(test)) {
                for (size_t i = 0; i < data.size(); ++i) data[i] ^= key[i % keyLen];
                return true;
            }
        }

        // Position-dependent XOR
        {
            uint8_t key4[4];
            for (int i = 0; i < 4; ++i) key4[i] = target[i] ^ data[i];
            std::vector<uint8_t> test(data.begin(), data.end());
            for (size_t i = 0; i < test.size(); ++i) test[i] ^= key4[i % 4] ^ static_cast<uint8_t>(i);
            if (test[0] == target[0] && test[1] == target[1] &&
                test[2] == target[2] && test[3] == target[3] &&
                isValidMetadataVersion(test)) {
                for (size_t i = 0; i < data.size(); ++i) data[i] ^= key4[i % 4] ^ static_cast<uint8_t>(i);
                return true;
            }
        }

        // Masked position XOR
        {
            uint8_t key4[4];
            for (int i = 0; i < 4; ++i) key4[i] = target[i] ^ data[i];
            std::vector<uint8_t> test(data.begin(), data.end());
            for (size_t i = 0; i < test.size(); ++i) test[i] ^= key4[i % 4] ^ static_cast<uint8_t>(i & 0xFF);
            if (test[0] == target[0] && test[1] == target[1] &&
                test[2] == target[2] && test[3] == target[3] &&
                isValidMetadataVersion(test)) {
                for (size_t i = 0; i < data.size(); ++i) data[i] ^= key4[i % 4] ^ static_cast<uint8_t>(i & 0xFF);
                return true;
            }
        }

        // Header-only XOR (256 bytes)
        {
            uint8_t key4[4];
            for (int i = 0; i < 4; ++i) key4[i] = target[i] ^ data[i];
            size_t headerSize = std::min<size_t>(256, data.size());
            std::vector<uint8_t> test(data.begin(), data.end());
            for (size_t i = 0; i < headerSize; ++i) test[i] ^= key4[i % 4];
            if (test[0] == target[0] && test[1] == target[1] &&
                test[2] == target[2] && test[3] == target[3] &&
                isValidMetadataVersion(test)) {
                for (size_t i = 0; i < headerSize; ++i) data[i] ^= key4[i % 4];
                return true;
            }
        }

        return false;
    }

    static bool isValidMetadataVersion(const std::vector<uint8_t>& data) {
        if (data.size() < 8) return false;
        int32_t ver;
        std::memcpy(&ver, data.data() + 4, 4);
        return ver > 0 && ver < 200;
    }

    // ── Metadata Parsing ──
    // profile->offsetOf() returns HEADER FIELD POSITIONS (where the value is stored).
    // We must read the uint32_t AT those positions to get the actual data offsets/counts.
    static void parseMetadata(const std::vector<uint8_t>& data,
                              const std::shared_ptr<IEngineProfile>& profile,
                              DumpData& result,
                              ProgressCallback progress) {
        // Read actual offsets and counts from metadata header.
        // profile->offsetOf() gives us the header byte positions.
        size_t typeDefFileOffset = static_cast<size_t>(
            readU32(data, profile->offsetOf("typeDefinitionsOffset")));
        uint32_t typeDefCount = readU32(data, profile->offsetOf("typeDefinitionCount"));
        size_t methodDefFileOffset = static_cast<size_t>(
            readU32(data, profile->offsetOf("methodDefinitionOffset")));
        uint32_t methodDefCount = readU32(data, profile->offsetOf("methodDefinitionCount"));
        size_t fieldDefFileOffset = static_cast<size_t>(
            readU32(data, profile->offsetOf("fieldDefinitionOffset")));
        uint32_t fieldDefCount = readU32(data, profile->offsetOf("fieldDefinitionCount"));
        size_t stringLiteralFileOffset = static_cast<size_t>(
            readU32(data, profile->offsetOf("stringLiteralOffset")));
        size_t stringLiteralDataFileOffset = static_cast<size_t>(
            readU32(data, profile->offsetOf("stringLiteralDataOffset")));

        size_t typeDefStructSize = profile->structSize("Il2CppTypeDefinition");
        size_t methodDefStructSize = profile->structSize("Il2CppMethodDefinition");
        size_t fieldDefStructSize = profile->structSize("Il2CppFieldDefinition");

        // Parse string literals first (needed for name resolution).
        if (progress) progress("Parsing string literals...");
        if (stringLiteralFileOffset > 0 && stringLiteralDataFileOffset > 0) {
            parseStringLiterals(data, stringLiteralFileOffset,
                                stringLiteralDataFileOffset, result);
        }

        // Parse type/method/field definitions with name resolution via string data.
        if (progress) progress("Parsing type definitions...");
        if (typeDefFileOffset > 0 && typeDefCount > 0 && typeDefStructSize > 0) {
            parseTypeDefinitions(data, typeDefFileOffset, typeDefCount,
                                 typeDefStructSize, stringLiteralDataFileOffset, result);
        }

        if (progress) progress("Parsing method definitions...");
        if (methodDefFileOffset > 0 && methodDefCount > 0 && methodDefStructSize > 0) {
            parseMethodDefinitions(data, methodDefFileOffset, methodDefCount,
                                   methodDefStructSize, stringLiteralDataFileOffset, result);
        }

        if (progress) progress("Parsing field definitions...");
        if (fieldDefFileOffset > 0 && fieldDefCount > 0 && fieldDefStructSize > 0) {
            parseFieldDefinitions(data, fieldDefFileOffset, fieldDefCount,
                                  fieldDefStructSize, stringLiteralDataFileOffset, result);
        }

        // Resolve method declaring types now that typeTable is populated.
        resolveMethodDeclaringTypes(result);
        resolveFieldDeclaringTypes(result);
    }

    // Parse TypeDefinitions. nameIndex is at offset 0x00 in each struct entry.
    static void parseTypeDefinitions(const std::vector<uint8_t>& data,
                                       size_t offset, uint32_t count,
                                       size_t structSize, size_t stringDataOffset,
                                       DumpData& result) {
        for (uint32_t i = 0; i < count; ++i) {
            size_t entryOff = offset + (i * structSize);
            if (entryOff + structSize > data.size()) break;

            TypeEntry type;
            type.typeId = i;
            type.address = entryOff;

            // nameIndex at offset 0x00 — resolve from string data blob.
            uint32_t nameIndex = readU32(data, entryOff);
            type.name = readNameFromData(data, stringDataOffset, nameIndex);
            if (type.name.empty()) {
                type.name = "Type_" + std::to_string(i);
            }

            // namespaceIndex at offset 0x04 (not stored in TypeEntry, but could be used later).
            // parentIndex at offset 0x14 (v24): index into TypeDef table.
            if (structSize >= 0x18) {
                uint32_t parentIdx = readU32(data, entryOff + 0x14);
                if (parentIdx < count) {
                    // Will be resolved in a second pass if needed.
                    type.parentType = "Type_" + std::to_string(parentIdx);
                }
            }

            result.typeTable.push_back(type);
        }
    }

    // Parse MethodDefinitions. nameIndex at 0x00, declaringTypeIndex at 0x04.
    static void parseMethodDefinitions(const std::vector<uint8_t>& data,
                                         size_t offset, uint32_t count,
                                         size_t structSize, size_t stringDataOffset,
                                         DumpData& result) {
        for (uint32_t i = 0; i < count; ++i) {
            size_t entryOff = offset + (i * structSize);
            if (entryOff + structSize > data.size()) break;

            MethodEntry method;
            method.methodIndex = i;

            // nameIndex at offset 0x00 — resolve from string data blob.
            uint32_t nameIndex = readU32(data, entryOff);
            method.name = readNameFromData(data, stringDataOffset, nameIndex);
            if (method.name.empty()) {
                method.name = "Method_" + std::to_string(i);
            }

            // declaringTypeIndex at offset 0x04 (index into TypeDef table).
            uint32_t declaringTypeIndex = readU32(data, entryOff + 0x04);
            method.declaringType = "Type_" + std::to_string(declaringTypeIndex);

            result.methodTable.push_back(method);
        }
    }

    // Parse FieldDefinitions. nameIndex at 0x00, typeIndex at 0x04, parentIndex at 0x08.
    static void parseFieldDefinitions(const std::vector<uint8_t>& data,
                                        size_t offset, uint32_t count,
                                        size_t structSize, size_t stringDataOffset,
                                        DumpData& result) {
        for (uint32_t i = 0; i < count; ++i) {
            size_t entryOff = offset + (i * structSize);
            if (entryOff + structSize > data.size()) break;

            FieldEntry field;

            // nameIndex at offset 0x00 — resolve from string data blob.
            uint32_t nameIndex = readU32(data, entryOff);
            field.name = readNameFromData(data, stringDataOffset, nameIndex);
            if (field.name.empty()) {
                field.name = "Field_" + std::to_string(i);
            }

            // typeIndex at offset 0x04 (index into TypeDef/encoded type).
            // parentIndex at offset 0x08 (index into TypeDef table).
            uint32_t parentIndex = readU32(data, entryOff + 0x08);
            field.declaringType = "Type_" + std::to_string(parentIndex);

            result.fieldTable.push_back(field);
        }
    }

    // Parse string literal table: Il2CppStringLiteral entries {index, length}.
    // The index is a byte offset into the string data region at stringLiteralDataOffset.
    static void parseStringLiterals(const std::vector<uint8_t>& data,
                                     size_t tableOffset, size_t dataOffset,
                                     DumpData& result) {
        if (tableOffset + 4 > data.size()) return;

        uint32_t count = readU32(data, tableOffset);
        size_t pos = tableOffset + 4;

        for (uint32_t i = 0; i < count && pos + 8 <= data.size(); ++i) {
            uint32_t length = readU32(data, pos);
            uint32_t dataIdx = readU32(data, pos + 4);
            pos += 8;

            StringEntry entry;
            size_t strAbsOffset = dataOffset + dataIdx;
            entry.address = strAbsOffset;

            if (strAbsOffset + length <= data.size() && length < 1024 * 1024) {
                entry.value = std::string(
                    reinterpret_cast<const char*>(data.data() + strAbsOffset), length);
            }

            result.stringTable.push_back(entry);
        }
    }

    // Resolve method declaring types using the populated typeTable.
    static void resolveMethodDeclaringTypes(DumpData& result) {
        for (auto& method : result.methodTable) {
            // declaringType is "Type_N" where N is the index. Look up real name.
            uint32_t idx = 0;
            const std::string& dt = method.declaringType;
            if (dt.size() > 5 && dt.compare(0, 5, "Type_") == 0) {
                idx = static_cast<uint32_t>(std::stoul(dt.substr(5)));
            }
            if (idx < result.typeTable.size()) {
                method.declaringType = result.typeTable[idx].name;
            }
        }
    }

    // Resolve field declaring types using the populated typeTable.
    static void resolveFieldDeclaringTypes(DumpData& result) {
        for (auto& field : result.fieldTable) {
            uint32_t idx = 0;
            const std::string& dt = field.declaringType;
            if (dt.size() > 5 && dt.compare(0, 5, "Type_") == 0) {
                idx = static_cast<uint32_t>(std::stoul(dt.substr(5)));
            }
            if (idx < result.typeTable.size()) {
                field.declaringType = result.typeTable[idx].name;
            }
        }
    }
};

} // namespace omnibyte::dumper::dumper1
