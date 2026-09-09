#pragma once
// Dumper1 — IL2CPP metadata parser (adapted from il2cpp-dumper-rs).
// Parses global-metadata.dat with XOR decryption, extracts TypeDefs, MethodDefs,
// FieldDefs, and string literals. No live process required.
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
    // Callback for progress updates during long operations.
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

        if (fileData.size() < 16) {
            result.errorMessage = "File too small to be IL2CPP metadata";
            return result;
        }

        if (progress) progress("Validating metadata magic...");

        uint32_t magic = readU32(fileData, 0);
        if (magic != kIl2CppMagic) {
            // Try decryption (adapted from il2cpp-dumper-rs try_decrypt_metadata).
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

        // Parse metadata using profile offsets.
        parseMetadata(fileData, profile, result, progress);

        result.setMeta("typeCount", std::to_string(result.typeTable.size()));
        result.setMeta("methodCount", std::to_string(result.methodTable.size()));
        result.setMeta("fieldCount", std::to_string(result.fieldTable.size()));
        result.setMeta("stringCount", std::to_string(result.stringTable.size()));
        result.success = true;
        return result;
    }

private:
    static const uint32_t kIl2CppMagic = 0xFAB11BAF; // v24+ global-metadata.dat magic

    static uint32_t readU32(const std::vector<uint8_t>& buf, size_t off) {
        if (off + 4 > buf.size()) return 0;
        uint32_t v;
        std::memcpy(&v, buf.data() + off, 4);
        return v;
    }

    static uint16_t readU16(const std::vector<uint8_t>& buf, size_t off) {
        if (off + 2 > buf.size()) return 0;
        uint16_t v;
        std::memcpy(&v, buf.data() + off, 2);
        return v;
    }

    // ── XOR Decryption (from il2cpp-dumper-rs try_decrypt_metadata) ──
    // Attempts single-byte, 4-byte, 8-byte, rolling, position-dependent, masked,
    // and header-only XOR schemes against the metadata.
    static bool tryDecryptMetadata(std::vector<uint8_t>& data) {
        if (data.size() < 16) return false;

        const uint8_t target[] = { 0xAF, 0x1B, 0xB1, 0xFA }; // LE bytes of 0xFAB11BAF

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
    static void parseMetadata(const std::vector<uint8_t>& data,
                              const std::shared_ptr<IEngineProfile>& profile,
                              DumpData& result,
                              ProgressCallback progress) {
        size_t typeDefOffset = static_cast<size_t>(profile->offsetOf("typeDefinitionsOffset"));
        uint32_t typeDefCount = static_cast<uint32_t>(profile->offsetOf("typeDefinitionCount"));
        size_t methodDefOffset = static_cast<size_t>(profile->offsetOf("methodDefinitionOffset"));
        uint32_t methodDefCount = static_cast<uint32_t>(profile->offsetOf("methodDefinitionCount"));
        size_t fieldDefOffset = static_cast<size_t>(profile->offsetOf("fieldDefinitionOffset"));
        uint32_t fieldDefCount = static_cast<uint32_t>(profile->offsetOf("fieldDefinitionCount"));
        size_t stringLiteralOffset = static_cast<size_t>(profile->offsetOf("stringLiteralOffset"));
        size_t stringLiteralDataOffset = static_cast<size_t>(profile->offsetOf("stringLiteralDataOffset"));

        size_t typeDefStructSize = profile->structSize("Il2CppTypeDefinition");
        size_t methodDefStructSize = profile->structSize("Il2CppMethodDefinition");
        size_t fieldDefStructSize = profile->structSize("Il2CppFieldDefinition");

        if (progress) progress("Parsing type definitions...");
        if (typeDefOffset > 0 && typeDefCount > 0 && typeDefStructSize > 0) {
            parseTypeDefinitions(data, typeDefOffset, typeDefCount, typeDefStructSize, result);
        }

        if (progress) progress("Parsing method definitions...");
        if (methodDefOffset > 0 && methodDefCount > 0 && methodDefStructSize > 0) {
            parseMethodDefinitions(data, methodDefOffset, methodDefCount, methodDefStructSize, result);
        }

        if (progress) progress("Parsing field definitions...");
        if (fieldDefOffset > 0 && fieldDefCount > 0 && fieldDefStructSize > 0) {
            parseFieldDefinitions(data, fieldDefOffset, fieldDefCount, fieldDefStructSize, result);
        }

        if (progress) progress("Parsing string literals...");
        if (stringLiteralOffset > 0 && stringLiteralDataOffset > 0) {
            parseStringLiterals(data, stringLiteralOffset, stringLiteralDataOffset, result);
        }
    }

    static void parseTypeDefinitions(const std::vector<uint8_t>& data,
                                      size_t offset, uint32_t count,
                                      size_t structSize, DumpData& result) {
        for (uint32_t i = 0; i < count; ++i) {
            size_t entryOff = offset + (i * structSize);
            if (entryOff + structSize > data.size()) break;

            TypeEntry type;
            type.typeId = i;
            type.name = "Type_" + std::to_string(i);
            type.address = entryOff;

            if (structSize >= 0x24) {
                uint16_t methodCount = static_cast<uint16_t>(
                    readU32(data, entryOff + 0x22) & 0xFFFF);
                type.size = methodCount;
            }

            result.typeTable.push_back(type);
        }
    }

    static void parseMethodDefinitions(const std::vector<uint8_t>& data,
                                        size_t offset, uint32_t count,
                                        size_t structSize, DumpData& result) {
        for (uint32_t i = 0; i < count; ++i) {
            size_t entryOff = offset + (i * structSize);
            if (entryOff + structSize > data.size()) break;

            MethodEntry method;
            method.methodIndex = i;
            method.name = "Method_" + std::to_string(i);

            uint32_t declaringTypeIndex = readU32(data, entryOff + 0x04);
            method.declaringType = "Type_" + std::to_string(declaringTypeIndex);

            result.methodTable.push_back(method);
        }
    }

    static void parseFieldDefinitions(const std::vector<uint8_t>& data,
                                       size_t offset, uint32_t count,
                                       size_t structSize, DumpData& result) {
        for (uint32_t i = 0; i < count; ++i) {
            size_t entryOff = offset + (i * structSize);
            if (entryOff + structSize > data.size()) break;

            FieldEntry field;
            field.name = "Field_" + std::to_string(i);

            uint32_t parentIndex = readU32(data, entryOff + 0x08);
            field.declaringType = "Type_" + std::to_string(parentIndex);

            result.fieldTable.push_back(field);
        }
    }

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
            entry.address = dataOffset + dataIdx;

            size_t strAbsOffset = dataOffset + dataIdx;
            if (strAbsOffset + length <= data.size()) {
                entry.value = std::string(
                    reinterpret_cast<const char*>(data.data() + strAbsOffset), length);
            }

            result.stringTable.push_back(entry);
        }
    }
};

} // namespace omnibyte::dumper::dumper1
