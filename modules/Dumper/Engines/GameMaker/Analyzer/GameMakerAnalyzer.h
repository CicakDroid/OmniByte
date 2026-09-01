#pragma once
// GameMaker — static analyzer for data.win chunk files.
// Parses FORM/YYYG magic, GEN8/STRG/OBJT/CODE/VARI/FUNC chunks.
// Does not require live process.
#include "../../../DumperCore/IDumperEngine.h"
#include "../../../DumperCore/IEngineProfile.h"
#include "../../../DumperCore/SharedUtils/SharedUtils.h"
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

namespace omnibyte::dumper::gamemaker {

class GameMakerAnalyzer {
public:
    static DumpResult analyze(const AnalysisTarget& target,
                              const std::shared_ptr<IEngineProfile>& profile) {
        DumpResult result;
        result.engineName = "GameMaker";
        result.detectedVersion = profile ? profile->version() : "unknown";

        if (!target.isFile()) {
            result.errorMessage = "GameMaker analyzer only supports file targets (data.win)";
            return result;
        }

        auto fileData = utils::readFileBytes(target.filePath);
        if (fileData.empty()) {
            result.errorMessage = "Failed to read file: " + target.filePath;
            return result;
        }

        if (fileData.size() < 12) {
            result.errorMessage = "File too small to be a valid data.win";
            return result;
        }

        // Validate magic: "YYYG" (GM 2.3+) or "FORM" (GMS1)
        uint32_t magic = readU32(fileData, 0);
        bool isGM23 = (magic == kMagicYYYG);
        bool isGMS1 = (magic == kMagicFORM);

        if (!isGM23 && !isGMS1) {
            result.errorMessage = "Invalid data.win magic (expected YYYG or FORM)";
            return result;
        }

        result.setMeta("format", isGM23 ? "GM2.3+" : "GMS1");

        // Read chunk size
        uint32_t fileSize = readU32(fileData, 4);
        result.setMeta("declaredFileSize", std::to_string(fileSize));

        // Parse chunks — iterate from offset 8
        size_t pos = 8;
        while (pos + 8 <= fileData.size()) {
            uint32_t chunkName = readU32(fileData, pos);
            uint32_t chunkSize = readU32(fileData, pos + 4);
            pos += 8;

            if (chunkSize == 0 || chunkSize > fileData.size()) break;
            if (pos + chunkSize > fileData.size()) break;

            parseChunk(chunkName, fileData, pos, chunkSize, result);
            pos += chunkSize;
        }

        result.success = true;
        return result;
    }

private:
    static const uint32_t kMagicYYYG = 0x59595947;  // "YYYG"
    static const uint32_t kMagicFORM = 0x464F524D;   // "FORM"

    // Chunk name constants (read as u32, little-endian)
    static const uint32_t kChunkGEN8 = 0x384E4547;  // "GEN8"
    static const uint32_t kChunkSTRG = 0x47525453;  // "STRG"
    static const uint32_t kChunkOBJT = 0x544A424F;  // "OBJT"
    static const uint32_t kChunkCODE = 0x45444F43;  // "CODE"
    static const uint32_t kChunkVARI = 0x49524156;  // "VARI"
    static const uint32_t kChunkFUNC = 0x434E5546;  // "FUNC"

    static void parseChunk(uint32_t name, const std::vector<uint8_t>& data,
                            size_t offset, uint32_t size, DumpResult& result) {
        if (name == kChunkGEN8) {
            parseGen8(data, offset, size, result);
        } else if (name == kChunkSTRG) {
            parseStrg(data, offset, size, result);
        } else if (name == kChunkOBJT) {
            parseObjt(data, offset, size, result);
        } else if (name == kChunkFUNC) {
            parseFunc(data, offset, size, result);
        }
        // CODE and VARI skipped for now — they contain bytecode/variable info
    }

    // GEN8: Game metadata (version, filename, counts)
    static void parseGen8(const std::vector<uint8_t>& data, size_t offset,
                           uint32_t size, DumpResult& result) {
        if (size < 8) return;

        uint32_t gmVersion = readU32(data, offset);
        result.setMeta("gmVersion", std::to_string(gmVersion));

        // Read filename (u32 length + chars) at offset +4
        if (offset + 8 <= data.size()) {
            uint32_t nameLen = readU32(data, offset + 4);
            if (nameLen > 0 && nameLen < 512 && offset + 8 + nameLen <= data.size()) {
                std::string filename(
                    reinterpret_cast<const char*>(data.data() + offset + 8), nameLen);
                result.setMeta("filename", filename);
            }
        }
    }

    // STRG: String table — count + entries (id u32, len u32, chars)
    static void parseStrg(const std::vector<uint8_t>& data, size_t offset,
                           uint32_t size, DumpResult& result) {
        if (size < 4) return;

        uint32_t count = readU32(data, offset);
        size_t pos = offset + 4;

        for (uint32_t i = 0; i < count && pos + 8 <= offset + size; ++i) {
            uint32_t id = readU32(data, pos);
            uint32_t strLen = readU32(data, pos + 4);
            pos += 8;

            if (strLen > 0 && strLen < 65536 && pos + strLen <= offset + size) {
                std::string str(reinterpret_cast<const char*>(data.data() + pos), strLen);
                pos += strLen;

                StringEntry entry;
                entry.value = str;
                entry.address = offset;
                result.stringTable.push_back(entry);
            } else {
                break;
            }
        }

        result.setMeta("stringCount", std::to_string(result.stringTable.size()));
    }

    // OBJT: Object definitions — count + entries (name, spriteId, visible, parentId)
    static void parseObjt(const std::vector<uint8_t>& data, size_t offset,
                           uint32_t size, DumpResult& result) {
        if (size < 4) return;

        uint32_t count = readU32(data, offset);
        size_t pos = offset + 4;

        for (uint32_t i = 0; i < count && pos + 4 <= offset + size; ++i) {
            // Read object name (u32 index into STRG table)
            uint32_t nameIndex = readU32(data, pos);

            TypeEntry type;
            type.typeId = i;
            type.name = "Object_" + std::to_string(i);  // placeholder, resolved via STRG
            type.address = pos;
            result.typeTable.push_back(type);

            // Skip object fields (spriteId, visible, solidity, parentId, depth)
            // Total per-object size varies by GM version, skip ~36 bytes
            pos += 36;
        }

        result.setMeta("objectCount", std::to_string(result.typeTable.size()));
    }

    // FUNC: Function names + code offsets
    static void parseFunc(const std::vector<uint8_t>& data, size_t offset,
                           uint32_t size, DumpResult& result) {
        if (size < 4) return;

        uint32_t count = readU32(data, offset);
        size_t pos = offset + 4;

        for (uint32_t i = 0; i < count && pos + 8 <= offset + size; ++i) {
            uint32_t nameIndex = readU32(data, pos);
            uint32_t codeOffset = readU32(data, pos + 4);
            pos += 8;

            MethodEntry method;
            method.name = "Func_" + std::to_string(i);  // placeholder, resolved via STRG
            method.methodIndex = i;
            method.address = codeOffset;
            result.methodTable.push_back(method);
        }

        result.setMeta("functionCount", std::to_string(result.methodTable.size()));
    }

    static uint32_t readU32(const std::vector<uint8_t>& buf, size_t off) {
        if (off + 4 > buf.size()) return 0;
        uint32_t v;
        std::memcpy(&v, buf.data() + off, 4);
        return v;
    }
};

// Static chunk name constants
const uint32_t GameMakerAnalyzer::kChunkGEN8;
const uint32_t GameMakerAnalyzer::kChunkSTRG;
const uint32_t GameMakerAnalyzer::kChunkOBJT;
const uint32_t GameMakerAnalyzer::kChunkCODE;
const uint32_t GameMakerAnalyzer::kChunkVARI;
const uint32_t GameMakerAnalyzer::kChunkFUNC;

} // namespace omnibyte::dumper::gamemaker
