// Boost.cpp -- OmniByte Boost wrapper for reverse engineering operations.
// BUILD NOTE: Requires CMake FetchContent for Boost headers.
// See Boost.h for details on standalone syntax check limitation.

#include "Boost.h"

#include <android/log.h>
#include <sstream>
#include <iomanip>
#include <algorithm>

#define LOG_TAG "Boost"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace omnibyte::common {

uint32_t boost_crc32(const uint8_t* data, size_t len) {
    boost::crc_optimal<32, 0x04C11DB7, 0xFFFFFFFF, 0xFFFFFFFF, true, true> crc;
    crc.process_block(data, data + len);
    return crc.checksum();
}

uint32_t boost_crc32c(const uint8_t* data, size_t len) {
    boost::crc_optimal<32, 0x1EDC6F41, 0xFFFFFFFF, 0xFFFFFFFF, true, true> crc;
    crc.process_block(data, data + len);
    return crc.checksum();
}

uint16_t boost_crc16_ccitt(const uint8_t* data, size_t len) {
    boost::crc_optimal<16, 0x1021, 0xFFFF, 0x0000, false, false> crc;
    crc.process_block(data, data + len);
    return crc.checksum();
}

uint64_t boost_hash_data(const uint8_t* data, size_t len) {
    boost::hash<const uint8_t*> hasher;
    return hasher(data);
}

uint64_t boost_hash_string(const std::string& str) {
    boost::hash<std::string> hasher;
    return hasher(str);
}

std::vector<size_t> boost_pattern_scan(
    const uint8_t* data, size_t dataLen,
    const uint8_t* pattern, size_t patternLen
) {
    std::vector<size_t> results;
    if (patternLen == 0 || dataLen < patternLen) return results;

    for (size_t i = 0; i <= dataLen - patternLen; ++i) {
        bool match = true;
        for (size_t j = 0; j < patternLen; ++j) {
            if (pattern[j] != 0xFF && data[i + j] != pattern[j]) {
                match = false;
                break;
            }
        }
        if (match) {
            results.push_back(i);
        }
    }
    return results;
}

std::string formatAddress(uintptr_t address, int width) {
    std::ostringstream oss;
    oss << "0x" << std::hex << std::setw(width) << std::setfill('0') << address;
    return oss.str();
}

std::string hexDump(const uint8_t* data, size_t len, size_t bytesPerLine) {
    std::ostringstream oss;
    for (size_t i = 0; i < len; i += bytesPerLine) {
        oss << formatAddress(i, 8) << "  ";
        // Hex bytes
        for (size_t j = 0; j < bytesPerLine; ++j) {
            if (i + j < len) {
                oss << std::hex << std::setw(2) << std::setfill('0')
                    << static_cast<int>(data[i + j]) << " ";
            } else {
                oss << "   ";
            }
        }
        oss << " |";
        // ASCII representation
        for (size_t j = 0; j < bytesPerLine && (i + j) < len; ++j) {
            uint8_t c = data[i + j];
            oss << (c >= 0x20 && c <= 0x7E ? static_cast<char>(c) : '.');
        }
        oss << "|\n";
    }
    return oss.str();
}

std::vector<uint8_t> hexToBytes(const std::string& hex) {
    std::vector<uint8_t> bytes;
    std::string clean = hex;
    boost::erase_all(clean, " ");
    boost::erase_all(clean, "0x");
    boost::erase_all(clean, "\n");

    if (clean.size() % 2 != 0) return bytes;

    for (size_t i = 0; i < clean.size(); i += 2) {
        uint8_t byte = static_cast<uint8_t>(
            std::stoul(clean.substr(i, 2), nullptr, 16)
        );
        bytes.push_back(byte);
    }
    return bytes;
}

std::string extractBetween(const std::string& str,
                           const std::string& start,
                           const std::string& end) {
    auto startPos = str.find(start);
    if (startPos == std::string::npos) return "";
    startPos += start.size();

    auto endPos = str.find(end, startPos);
    if (endPos == std::string::npos) return str.substr(startPos);

    return str.substr(startPos, endPos - startPos);
}

} // namespace omnibyte::common
