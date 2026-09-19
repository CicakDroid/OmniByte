// SignatureExtractor — extract v1 (JAR) and v2/v3 signing blocks from APK files.

#include "SignatureExtractor.h"

#include <cstdio>
#include <cstring>
#include <fstream>

namespace omnibyte::runtime::backends {

// --- ZIP constants ---
#pragma pack(push, 1)
struct LocalFileHeader {
    uint32_t signature;       // 0x04034b50
    uint16_t versionNeeded;
    uint16_t flags;
    uint16_t compression;
    uint16_t modTime;
    uint16_t modDate;
    uint32_t crc32;
    uint32_t compressedSize;
    uint32_t uncompressedSize;
    uint16_t nameLen;
    uint16_t extraLen;
};

struct CentralDirHeader {
    uint32_t signature;       // 0x02014b50
    uint16_t versionMadeBy;
    uint16_t versionNeeded;
    uint16_t flags;
    uint16_t compression;
    uint16_t modTime;
    uint16_t modDate;
    uint32_t crc32;
    uint32_t compressedSize;
    uint32_t uncompressedSize;
    uint16_t nameLen;
    uint16_t extraLen;
    uint16_t commentLen;
    uint16_t diskStart;
    uint16_t internalAttr;
    uint32_t externalAttr;
    uint32_t localHeaderOffset;
};

struct EOCD {
    uint32_t signature;       // 0x06054b50
    uint16_t diskNum;
    uint16_t diskCdStart;
    uint16_t numEntriesDisk;
    uint16_t numEntriesTotal;
    uint32_t cdSize;
    uint32_t cdOffset;
    uint16_t commentLen;
};
#pragma pack(pop)

static constexpr uint32_t kLocalFileSig = 0x04034b50;
static constexpr uint32_t kCentralDirSig = 0x02014b50;
static constexpr uint32_t kEOCDSig = 0x06054b50;

// --- Helpers ---

uint32_t SignatureExtractor::readU32LE(const uint8_t* buf, size_t off) {
    return uint32_t(buf[off])       | (uint32_t(buf[off+1]) << 8) |
           (uint32_t(buf[off+2]) << 16) | (uint32_t(buf[off+3]) << 24);
}

int64_t SignatureExtractor::findEOCD(const uint8_t* data, size_t len) {
    // EOCD is at least 22 bytes. Search backwards for its magic.
    if (len < 22) return -1;
    for (size_t i = len - 22; i >= len && i < len; --i) {
        if (readU32LE(data, i) == kEOCDSig) {
            // Verify comment length fits.
            uint16_t commentLen = uint16_t(data[i+20]) | (uint16_t(data[i+21]) << 8);
            if (i + 22 + commentLen <= len) return static_cast<int64_t>(i);
        }
        if (i == 0) break;
    }
    return -1;
}

int64_t SignatureExtractor::findLastLocalFileHeader(const uint8_t* data, size_t len) {
    int64_t last = -1;
    size_t pos = 0;
    while (pos + sizeof(LocalFileHeader) <= len) {
        if (readU32LE(data, pos) != kLocalFileSig) break;
        auto* lfh = reinterpret_cast<const LocalFileHeader*>(data + pos);
        size_t entrySize = sizeof(LocalFileHeader) + lfh->nameLen + lfh->extraLen +
                           lfh->compressedSize;
        last = static_cast<int64_t>(pos);
        pos += entrySize;
        if (entrySize < sizeof(LocalFileHeader)) break; // corrupt
    }
    return last;
}

bool SignatureExtractor::isMetaInfEntry(const uint8_t* name, uint16_t nameLen) {
    static const char prefix[] = "META-INF/";
    static const size_t prefixLen = 9;
    return nameLen >= prefixLen && memcmp(name, prefix, prefixLen) == 0;
}

// --- Public API ---

bool SignatureExtractor::extractV1(const std::string& apkPath, std::string& outDir) {
    std::ifstream ifs(apkPath, std::ios::binary);
    if (!ifs) return false;

    // Read entire file into memory (APKs are typically <100MB).
    ifs.seekg(0, std::ios::end);
    auto fileSize = ifs.tellg();
    if (fileSize <= 0) return false;
    ifs.seekg(0);

    std::vector<uint8_t> buf(static_cast<size_t>(fileSize));
    ifs.read(reinterpret_cast<char*>(buf.data()), fileSize);
    ifs.close();

    const uint8_t* data = buf.data();
    const size_t len = buf.size();

    int64_t eocdOff = findEOCD(data, len);
    if (eocdOff < 0) return false;

    // Iterate Central Directory to find META-INF/ entries.
    auto* eocd = reinterpret_cast<const EOCD*>(data + eocdOff);
    uint32_t cdOffset = eocd->cdOffset;
    if (cdOffset >= len) return false;

    // Ensure outDir exists.
    std::string mkdirCmd = "mkdir -p \"" + outDir + "\"";
    if (system(mkdirCmd.c_str()) != 0) return false;

    bool found = false;
    size_t pos = cdOffset;
    for (uint16_t i = 0; i < eocd->numEntriesTotal; ++i) {
        if (pos + sizeof(CentralDirHeader) > len) break;
        if (readU32LE(data, pos) != kCentralDirSig) break;

        auto* cdh = reinterpret_cast<const CentralDirHeader*>(data + pos);
        const uint8_t* name = data + pos + sizeof(CentralDirHeader);
        uint16_t nameLen = cdh->nameLen;

        if (isMetaInfEntry(name, nameLen)) {
            // Extract file name as string.
            std::string entryName(reinterpret_cast<const char*>(name), nameLen);
            std::string outPath = outDir + "/" + entryName;

            // Read from local file header.
            uint32_t lfhOffset = cdh->localHeaderOffset;
            if (lfhOffset + sizeof(LocalFileHeader) > len) continue;
            auto* lfh = reinterpret_cast<const LocalFileHeader*>(data + lfhOffset);
            size_t dataOffset = lfhOffset + sizeof(LocalFileHeader) +
                                lfh->nameLen + lfh->extraLen;
            uint32_t dataLen = lfh->compressedSize;
            if (dataOffset + dataLen > len) continue;

            std::ofstream ofs(outPath, std::ios::binary);
            if (ofs) {
                ofs.write(reinterpret_cast<const char*>(data + dataOffset), dataLen);
                found = true;
            }
        }

        pos += sizeof(CentralDirHeader) + cdh->nameLen + cdh->extraLen + cdh->commentLen;
    }

    return found;
}

bool SignatureExtractor::extractV2V3(const std::string& apkPath,
                                     std::vector<uint8_t>& outSigningBlock) {
    std::ifstream ifs(apkPath, std::ios::binary);
    if (!ifs) return false;

    ifs.seekg(0, std::ios::end);
    auto fileSize = ifs.tellg();
    if (fileSize <= 0) return false;
    ifs.seekg(0);

    std::vector<uint8_t> buf(static_cast<size_t>(fileSize));
    ifs.read(reinterpret_cast<char*>(buf.data()), fileSize);
    ifs.close();

    const uint8_t* data = buf.data();
    const size_t len = buf.size();

    int64_t eocdOff = findEOCD(data, len);
    int64_t lastLfh = findLastLocalFileHeader(data, len);
    if (eocdOff < 0 || lastLfh < 0) return false;

    // EOCD starts at eocdOff. The signing block (if present) sits between
    // the end of the last local file entry and the start of EOCD.
    // But for v2/v3, the signing block is appended BEFORE EOCD in the APK.
    // The EOCD cdOffset points to the start of the Central Directory.
    // The signing block sits between the last local file + data and the EOCD.

    auto* eocd = reinterpret_cast<const EOCD*>(data + eocdOff);
    uint32_t cdOffset = eocd->cdOffset;

    // The signing block is between the end of last LFH entry and cdOffset.
    // Actually, the APK structure is: [local files] [central dir] [EOCD]
    // For v2/v3 signing, the block is inserted AFTER local files but BEFORE CD.
    // Wait — the standard says: [local files] [APK Signing Block] [Central Dir] [EOCD]
    // The EOCD.cdOffset points to Central Dir, and the signing block is right before it.
    // So we need to find the gap between end of last local file and cdOffset.

    // Calculate end of last local file entry.
    size_t endOfLastLfh = 0;
    {
        size_t pos = 0;
        while (pos + sizeof(LocalFileHeader) <= len) {
            if (readU32LE(data, pos) != kLocalFileSig) break;
            auto* lfh = reinterpret_cast<const LocalFileHeader*>(data + pos);
            size_t entrySize = sizeof(LocalFileHeader) + lfh->nameLen + lfh->extraLen +
                               lfh->compressedSize;
            pos += entrySize;
            if (entrySize < sizeof(LocalFileHeader)) break;
        }
        endOfLastLfh = pos;
    }

    // The signing block occupies the space between endOfLastLfh and cdOffset.
    // Its last 16 bytes are: uint64 size (of pairs) + magic (8 bytes).
    size_t blockRegionStart = endOfLastLfh;
    size_t blockRegionSize = cdOffset - blockRegionStart;
    if (blockRegionSize < 24) return false; // minimum: 8 (size) + 8 (magic) + 8 (one pair header)

    const uint8_t* blockData = data + blockRegionStart;
    // Read the uint64 that stores the total size of (pairs + 8-byte-size).
    // Format: [uint64 pairSize] [pairs...] [uint32 blockSize] [uint32 magic]
    // Actually: block = [uint64 pairContentSize] [pair1] [pair2] ... [uint32 blockSize] [uint32 magic]
    // where blockSize = total block size excluding blockSize+magic (so pairContentSize + 8)
    // and magic = kV2Magic or kV3Magic.

    // The last 8 bytes of the block region are: uint32 blockSize + uint32 magic
    size_t magicOff = blockRegionStart + blockRegionSize - 8;
    uint32_t blockMagic = readU32LE(data, magicOff);
    if (blockMagic != kV2Magic && blockMagic != kV3Magic) return false;

    // blockSize is at magicOff - 4. blockSize = total size excluding blockSize+magic
    uint32_t blockSize = readU32LE(data, magicOff - 4);
    // Full signing block = blockSize + 8 (for blockSize + magic)
    size_t fullBlockSize = static_cast<size_t>(blockSize) + 8;
    if (fullBlockSize > blockRegionSize) return false;

    size_t blockStart = magicOff + 8 - fullBlockSize;
    outSigningBlock.assign(data + blockStart, data + blockStart + fullBlockSize);
    return true;
}

bool SignatureExtractor::extractAll(const std::string& apkPath, const std::string& outDir) {
    bool v1 = extractV1(apkPath, outDir);
    std::vector<uint8_t> v2v3;
    bool v2v3ok = extractV2V3(apkPath, v2v3);
    if (v2v3ok) {
        std::string blockPath = outDir + "/signing_block.bin";
        std::ofstream ofs(blockPath, std::ios::binary);
        if (ofs) {
            ofs.write(reinterpret_cast<const char*>(v2v3.data()), v2v3.size());
        }
    }
    return v1 || v2v3ok;
}

SignatureInfo SignatureExtractor::getInfo(const std::string& apkPath) const {
    SignatureInfo info;

    // Check v1: try to find META-INF/ in the ZIP.
    {
        std::ifstream ifs(apkPath, std::ios::binary);
        if (ifs) {
            ifs.seekg(0, std::ios::end);
            auto fileSize = ifs.tellg();
            if (fileSize > 0) {
                ifs.seekg(0);
                std::vector<uint8_t> buf(static_cast<size_t>(fileSize));
                ifs.read(reinterpret_cast<char*>(buf.data()), fileSize);

                int64_t eocdOff = findEOCD(buf.data(), buf.size());
                if (eocdOff >= 0) {
                    auto* eocd = reinterpret_cast<const EOCD*>(buf.data() + eocdOff);
                    size_t pos = eocd->cdOffset;
                    for (uint16_t i = 0; i < eocd->numEntriesTotal; ++i) {
                        if (pos + sizeof(CentralDirHeader) > buf.size()) break;
                        if (readU32LE(buf.data(), pos) != kCentralDirSig) break;
                        auto* cdh = reinterpret_cast<const CentralDirHeader*>(buf.data() + pos);
                        if (isMetaInfEntry(buf.data() + pos + sizeof(CentralDirHeader),
                                           cdh->nameLen)) {
                            info.hasV1 = true;
                            break;
                        }
                        pos += sizeof(CentralDirHeader) + cdh->nameLen + cdh->extraLen +
                               cdh->commentLen;
                    }
                }
            }
        }
    }

    // Check v2/v3.
    std::vector<uint8_t> block;
    if (const_cast<SignatureExtractor*>(this)->extractV2V3(apkPath, block)) {
        if (block.size() >= 8) {
            uint32_t magic = readU32LE(block.data(), block.size() - 4);
            if (magic == kV2Magic) {
                info.hasV2 = true;
                info.apiLevel = 28;
            } else if (magic == kV3Magic) {
                info.hasV3 = true;
                info.apiLevel = 30;
            }
        }
    }

    return info;
}

} // namespace omnibyte::runtime::backends
