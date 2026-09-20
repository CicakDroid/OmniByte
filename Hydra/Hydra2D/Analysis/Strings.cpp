#include "Strings.h"

#include <algorithm>
#include <cctype>
#include <cstring>

namespace omnibyte::hydradis {

StringsResult Strings::analyzeStrings(
    const uint8_t* data, size_t dataSize
) const {
    StringsResult result;
    if (!data || dataSize == 0) {
        result.success = true;
        return result;
    }

    auto extracted = extractStrings(data, dataSize);

    for (auto& [addr, str] : extracted) {
        result.stringAddresses.push_back(addr);
        std::string category = classifyString(str);
        result.stringClassifications[addr] = category;
        result.classifiedStrings++;
    }

    result.success = true;
    return result;
}

std::vector<std::pair<uint64_t, std::string>> Strings::extractStrings(
    const uint8_t* data, size_t dataSize,
    size_t minLength
) const {
    std::vector<std::pair<uint64_t, std::string>> strings;

    size_t start = 0;
    bool inString = false;

    for (size_t i = 0; i <= dataSize; ++i) {
        bool isPrintable = (i < dataSize) &&
                           (std::isprint(data[i]) || data[i] == '\t' || data[i] == '\n');

        if (isPrintable && !inString) {
            start = i;
            inString = true;
        } else if (!isPrintable && inString) {
            size_t len = i - start;
            if (len >= minLength) {
                std::string str(reinterpret_cast<const char*>(data + start), len);
                strings.emplace_back(static_cast<uint64_t>(start), str);
            }
            inString = false;
        }
    }

    return strings;
}

std::string Strings::classifyString(const std::string& str) const {
    if (isFormatString(str)) return "format";
    if (isUrlString(str)) return "url";
    if (isHexString(str)) return "hex_constant";

    bool allAlpha = true;
    bool allDigit = true;
    for (char c : str) {
        if (!std::isalpha(c) && c != '_' && c != ' ') allAlpha = false;
        if (!std::isdigit(c)) allDigit = false;
    }

    if (allDigit) return "numeric_constant";
    if (allAlpha) return "identifier";

    bool hasPath = str.find('/') != std::string::npos ||
                   str.find('\\') != std::string::npos;
    if (hasPath && str.size() > 8) return "path";

    bool hasNulls = str.find('\0') != std::string::npos;
    if (hasNulls) return "embedded";

    return "string_literal";
}

bool Strings::isFormatString(const std::string& str) const {
    return str.find("%s") != std::string::npos ||
           str.find("%d") != std::string::npos ||
           str.find("%x") != std::string::npos ||
           str.find("%p") != std::string::npos ||
           str.find("%f") != std::string::npos ||
           str.find("%ld") != std::string::npos ||
           str.find("%zu") != std::string::npos;
}

bool Strings::isUrlString(const std::string& str) const {
    return str.find("http://") != std::string::npos ||
           str.find("https://") != std::string::npos ||
           str.find("ftp://") != std::string::npos ||
           str.find("www.") != std::string::npos;
}

bool Strings::isHexString(const std::string& str) const {
    if (str.size() < 2) return false;
    if (str[0] != '0' || (str[1] != 'x' && str[1] != 'X')) return false;
    for (size_t i = 2; i < str.size(); ++i) {
        char c = str[i];
        if (!std::isxdigit(c)) return false;
    }
    return true;
}

} // namespace omnibyte::hydradis
