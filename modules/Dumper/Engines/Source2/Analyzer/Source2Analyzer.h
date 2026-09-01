#pragma once
// Source2 — static analyzer for Valve Source 2 compiled resources.
// Parses resource block headers and schema definitions from .vpk_c files.
// Does not require live process.
#include "../../../DumperCore/IDumperEngine.h"
#include "../../../DumperCore/IEngineProfile.h"
#include "../../../DumperCore/SharedUtils/SharedUtils.h"
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

namespace omnibyte::dumper::source2 {

class Source2Analyzer {
public:
    static DumpResult analyze(const AnalysisTarget& target,
                              const std::shared_ptr<IEngineProfile>& profile) {
        DumpResult result;
        result.engineName = "Source2";
        result.detectedVersion = profile ? profile->version() : "unknown";

        if (!target.isFile()) {
            result.errorMessage = "Source2 analyzer only supports file targets (.vpk_c/.vpk)";
            return result;
        }

        auto fileData = utils::readFileBytes(target.filePath);
        if (fileData.empty()) {
            result.errorMessage = "Failed to read file: " + target.filePath;
            return result;
        }

        if (fileData.size() < kMinResourceSize) {
            result.errorMessage = "File too small to be a valid Source 2 resource";
            return result;
        }

        // Validate Source 2 resource magic
        // Source 2 uses multiple magic values depending on format version
        uint32_t magic = readU32(fileData, 0);
        if (!isValidSource2Magic(magic)) {
            result.errorMessage = "Invalid Source 2 resource magic";
            return result;
        }

        // Parse resource header
        uint32_t headerSize = readU32(fileData, 4);
        uint32_t resourceVersion = readU32(fileData, 8);

        result.setMeta("resourceVersion", std::to_string(resourceVersion));
        result.setMeta("headerSize", std::to_string(headerSize));

        // Walk schema definitions if header contains schema info
        size_t schemaOffset = headerSize;
        int classCount = 0;

        while (schemaOffset + 8 <= fileData.size()) {
            uint32_t schemaMagic = readU32(fileData, schemaOffset);
            uint32_t schemaSize = readU32(fileData, schemaOffset + 4);

            if (schemaSize == 0 || schemaSize > 1024 * 1024) break;  // sanity
            if (schemaOffset + schemaSize > fileData.size()) break;

            // Try to extract class name from schema block
            // Source 2 schemas are self-describing — class name is embedded
            size_t nameOffset = schemaOffset + 8;
            if (nameOffset + 4 <= schemaOffset + schemaSize) {
                uint32_t nameLen = readU32(fileData, nameOffset);
                if (nameLen > 0 && nameLen < 512 &&
                    nameOffset + 4 + nameLen <= schemaOffset + schemaSize) {
                    std::string className(
                        reinterpret_cast<const char*>(fileData.data() + nameOffset + 4),
                        nameLen);

                    TypeEntry type;
                    type.name = className;
                    type.address = schemaOffset;
                    result.typeTable.push_back(type);
                    classCount++;
                }
            }

            schemaOffset += schemaSize;
        }

        result.setMeta("classCount", std::to_string(classCount));
        result.success = true;
        return result;
    }

private:
    static constexpr size_t kMinResourceSize = 16;

    // Source 2 uses multiple magic values
    static bool isValidSource2Magic(uint32_t magic) {
        // ResourceSystem compiled resource magic
        static const uint32_t kValidMagics[] = {
            0x52455300,  // "RES\0" (Resource)
            0x56504B00,  // "VPK\0" (VPK container)
            0x56455200,  // "VER\0" (Version block)
        };
        for (uint32_t m : kValidMagics) {
            if (magic == m) return true;
        }
        return false;
    }

    static uint32_t readU32(const std::vector<uint8_t>& buf, size_t off) {
        if (off + 4 > buf.size()) return 0;
        uint32_t v;
        std::memcpy(&v, buf.data() + off, 4);
        return v;
    }
};

} // namespace omnibyte::dumper::source2
