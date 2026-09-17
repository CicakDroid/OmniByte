// SignatureInjector — inject v1 (JAR) and v2/v3 signing blocks into APK files.

#include "SignatureInjector.h"
#include "SignatureExtractor.h"

#include <cstdio>
#include <cstring>
#include <fstream>

namespace omnibyte::runtime::backends {

// --- ZIP structures ---

#pragma pack(push, 1)
struct LocalFileHeader {
    uint32_t signature;
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
    uint32_t signature;
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
    uint32_t signature;
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

uint32_t SignatureInjector::readU32LE(const uint8_t* buf, size_t off) {
    return uint32_t(buf[off]) | (uint32_t(buf[off+1]) << 8) |
           (uint32_t(buf[off+2]) << 16) | (uint32_t(buf[off+3]) << 24);
}

void SignatureInjector::writeU32LE(uint8_t* buf, uint32_t val) {
    buf[0] = static_cast<uint8_t>(val);
    buf[1] = static_cast<uint8_t>(val >> 8);
    buf[2] = static_cast<uint8_t>(val >> 16);
    buf[3] = static_cast<uint8_t>(val >> 24);
}

int64_t SignatureInjector::findEOCD(const uint8_t* data, size_t len) {
    if (len < 22) return -1;
    for (size_t i = len - 22; i >= len && i < len; --i) {
        if (readU32LE(data, i) == kEOCDSig) {
            uint16_t commentLen = uint16_t(data[i+20]) | (uint16_t(data[i+21]) << 8);
            if (i + 22 + commentLen <= len) return static_cast<int64_t>(i);
        }
        if (i == 0) break;
    }
    return -1;
}

int64_t SignatureInjector::findLastLocalFileHeader(const uint8_t* data, size_t len) {
    int64_t last = -1;
    size_t pos = 0;
    while (pos + sizeof(LocalFileHeader) <= len) {
        if (readU32LE(data, pos) != kLocalFileSig) break;
        auto* lfh = reinterpret_cast<const LocalFileHeader*>(data + pos);
        size_t entrySize = sizeof(LocalFileHeader) + lfh->nameLen + lfh->extraLen +
                           lfh->compressedSize;
        last = static_cast<int64_t>(pos);
        pos += entrySize;
        if (entrySize < sizeof(LocalFileHeader)) break;
    }
    return last;
}

bool SignatureInjector::appendZipEntry(const std::string& outPath,
                                       const std::string& entryName,
                                       const uint8_t* data, size_t dataLen) {
    std::ofstream ofs(outPath, std::ios::binary | std::ios::app);
    if (!ofs) return false;

    uint32_t nameLen = static_cast<uint32_t>(entryName.size());

    // Local File Header.
    LocalFileHeader lfh{};
    lfh.signature = kLocalFileSig;
    lfh.versionNeeded = 20;
    lfh.flags = 0;
    lfh.compression = 0; // stored
    lfh.crc32 = 0; // not computing CRC for signature files
    lfh.compressedSize = static_cast<uint32_t>(dataLen);
    lfh.uncompressedSize = static_cast<uint32_t>(dataLen);
    lfh.nameLen = nameLen;
    lfh.extraLen = 0;

    ofs.write(reinterpret_cast<const char*>(&lfh), sizeof(lfh));
    ofs.write(entryName.c_str(), nameLen);
    if (data && dataLen > 0) {
        ofs.write(reinterpret_cast<const char*>(data), dataLen);
    }

    return ofs.good();
}

bool SignatureInjector::appendRaw(const std::string& path, const uint8_t* data, size_t len) {
    std::ofstream ofs(path, std::ios::binary | std::ios::app);
    if (!ofs) return false;
    ofs.write(reinterpret_cast<const char*>(data), len);
    return ofs.good();
}

bool SignatureInjector::copyFile(const std::string& src, const std::string& dst) {
    std::ifstream ifs(src, std::ios::binary);
    if (!ifs) return false;
    std::ofstream ofs(dst, std::ios::binary);
    if (!ofs) return false;
    ofs << ifs.rdbuf();
    return ofs.good();
}

// --- Public API ---

bool SignatureInjector::injectV1(const std::string& srcApk, const std::string& signDir,
                                 const std::string& outApk) {
    // Copy source APK to output.
    if (!copyFile(srcApk, outApk)) return false;

    // List META-INF files in signDir.
    std::string listCmd = "ls \"" + signDir + "/META-INF/\" 2>/dev/null";
    FILE* pipe = popen(listCmd.c_str(), "r");
    if (!pipe) return false;

    char buf[512];
    bool anyInjected = false;
    while (fgets(buf, sizeof(buf), pipe)) {
        // Trim newline.
        size_t len = strlen(buf);
        while (len > 0 && (buf[len-1] == '\n' || buf[len-1] == '\r')) buf[--len] = '\0';
        if (len == 0) continue;

        std::string fileName = buf;
        std::string filePath = signDir + "/META-INF/" + fileName;
        std::string entryName = "META-INF/" + fileName;

        // Read the file.
        std::ifstream ifs(filePath, std::ios::binary);
        if (!ifs) continue;
        ifs.seekg(0, std::ios::end);
        auto fsize = ifs.tellg();
        if (fsize <= 0) continue;
        ifs.seekg(0);

        std::vector<uint8_t> fileData(static_cast<size_t>(fsize));
        ifs.read(reinterpret_cast<char*>(fileData.data()), fsize);
        ifs.close();

        // Append as ZIP entry.
        if (appendZipEntry(outApk, entryName, fileData.data(), fileData.size())) {
            anyInjected = true;
        }
    }
    pclose(pipe);

    if (!anyInjected) return false;

    // Rebuild Central Directory and EOCD.
    // Read the current APK to build CD.
    std::ifstream ifs(outApk, std::ios::binary);
    if (!ifs) return false;
    ifs.seekg(0, std::ios::end);
    auto fileSize = ifs.tellg();
    ifs.seekg(0);
    std::vector<uint8_t> apkBuf(static_cast<size_t>(fileSize));
    ifs.read(reinterpret_cast<char*>(apkBuf.data()), fileSize);
    ifs.close();

    // Count entries and build CD.
    std::vector<uint8_t> cd;
    uint16_t entryCount = 0;
    size_t pos = 0;
    while (pos + sizeof(LocalFileHeader) <= apkBuf.size()) {
        if (readU32LE(apkBuf.data(), pos) != kLocalFileSig) break;
        auto* lfh = reinterpret_cast<const LocalFileHeader*>(apkBuf.data() + pos);
        size_t entrySize = sizeof(LocalFileHeader) + lfh->nameLen + lfh->extraLen +
                           lfh->compressedSize;
        if (entrySize < sizeof(LocalFileHeader)) break;

        // Build Central Directory entry.
        CentralDirHeader cdh{};
        cdh.signature = kCentralDirSig;
        cdh.versionMadeBy = 20;
        cdh.versionNeeded = lfh->versionNeeded;
        cdh.flags = lfh->flags;
        cdh.compression = lfh->compression;
        cdh.modTime = lfh->modTime;
        cdh.modDate = lfh->modDate;
        cdh.crc32 = lfh->crc32;
        cdh.compressedSize = lfh->compressedSize;
        cdh.uncompressedSize = lfh->uncompressedSize;
        cdh.nameLen = lfh->nameLen;
        cdh.extraLen = lfh->extraLen;
        cdh.commentLen = 0;
        cdh.diskStart = 0;
        cdh.internalAttr = 0;
        cdh.externalAttr = 0;
        cdh.localHeaderOffset = static_cast<uint32_t>(pos);

        // Write CD entry.
        std::vector<uint8_t> cdEntry(sizeof(CentralDirHeader) + lfh->nameLen);
        memcpy(cdEntry.data(), &cdh, sizeof(CentralDirHeader));
        memcpy(cdEntry.data() + sizeof(CentralDirHeader),
               apkBuf.data() + pos + sizeof(LocalFileHeader), lfh->nameLen);
        cd.insert(cd.end(), cdEntry.begin(), cdEntry.end());

        entryCount++;
        pos += entrySize;
    }

    uint32_t cdOffset = static_cast<uint32_t>(pos);
    uint32_t cdSize = static_cast<uint32_t>(cd.size());

    // Append CD.
    std::ofstream ofs(outApk, std::ios::binary | std::ios::app);
    if (!ofs) return false;
    ofs.write(reinterpret_cast<const char*>(cd.data()), cd.size());

    // Write EOCD.
    EOCD eocd{};
    eocd.signature = kEOCDSig;
    eocd.numEntriesTotal = entryCount;
    eocd.cdSize = cdSize;
    eocd.cdOffset = cdOffset;
    ofs.write(reinterpret_cast<const char*>(&eocd), sizeof(eocd));

    return ofs.good();
}

bool SignatureInjector::injectV2V3(const std::string& srcApk,
                                   const std::vector<uint8_t>& signingBlock,
                                   const std::string& outApk) {
    // Copy source APK.
    if (!copyFile(srcApk, outApk)) return false;

    // Read current APK to find the insertion point.
    std::ifstream ifs(outApk, std::ios::binary);
    if (!ifs) return false;
    ifs.seekg(0, std::ios::end);
    auto fileSize = ifs.tellg();
    ifs.seekg(0);
    std::vector<uint8_t> buf(static_cast<size_t>(fileSize));
    ifs.read(reinterpret_cast<char*>(buf.data()), fileSize);
    ifs.close();

    int64_t eocdOff = SignatureExtractor::findEOCD(buf.data(), buf.size());
    if (eocdOff < 0) return false;

    // The signing block goes right before EOCD.
    // Actually, for v2/v3 the signing block is before EOCD but after CD.
    // We insert it before the EOCD. Then update EOCD.cdOffset += blockSize.

    auto* eocd = reinterpret_cast<const EOCD*>(buf.data() + eocdOff);
    uint32_t oldCdOffset = eocd->cdOffset;
    uint32_t blockSize = static_cast<uint32_t>(signingBlock.size());

    // Write: [signing block] [EOCD with updated cdOffset].
    // Truncate the file at eocdOff, then append block + updated EOCD.
    {
        std::ofstream ofs(outApk, std::ios::binary | std::ios::trunc);
        if (!ofs) return false;
        ofs.write(reinterpret_cast<const char*>(buf.data()), eocdOff);

        // Append signing block.
        ofs.write(reinterpret_cast<const char*>(signingBlock.data()), signingBlock.size());

        // Write updated EOCD.
        EOCD newEocd = *eocd;
        newEocd.cdOffset = oldCdOffset + blockSize;
        ofs.write(reinterpret_cast<const char*>(&newEocd), sizeof(newEocd));
    }

    return true;
}

bool SignatureInjector::cloneSignatures(const std::string& origApk,
                                        const std::string& modApk,
                                        const std::string& outApk) {
    // 1. Extract v1 META-INF/ from origApk to temp dir.
    std::string tmpDir = "/tmp/omnibyte_sig_" + std::to_string(reinterpret_cast<uintptr_t>(this));
    std::string mkdirCmd = "mkdir -p \"" + tmpDir + "\"";
    if (system(mkdirCmd.c_str()) != 0) return false;

    // Use SignatureExtractor to extract.
    SignatureExtractor extractor;
    bool hasV1 = extractor.extractV1(origApk, tmpDir);

    // 2. Extract v2/v3 block.
    std::vector<uint8_t> v2v3Block;
    bool hasV2V3 = extractor.extractV2V3(origApk, v2v3Block);

    // 3. Inject into modApk.
    if (hasV1) {
        if (!injectV1(modApk, tmpDir, outApk)) {
            // Fall back to copying modApk as-is.
            copyFile(modApk, outApk);
        }
    } else {
        if (!copyFile(modApk, outApk)) return false;
    }

    if (hasV2V3) {
        injectV2V3(outApk, v2v3Block, outApk);
    }

    // Cleanup temp dir.
    std::string rmCmd = "rm -rf \"" + tmpDir + "\"";
    system(rmCmd.c_str());

    return true;
}

} // namespace omnibyte::runtime::backends
