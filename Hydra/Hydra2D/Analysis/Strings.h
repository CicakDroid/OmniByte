#pragma once

#include "IAnalysis.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace omnibyte::hydradis {

class Strings : public IAnalysis {
public:
    std::string name() const override { return "Strings"; }

    StringsResult analyzeStrings(
        const uint8_t* data, size_t dataSize
    ) const override;

private:
    std::vector<std::pair<uint64_t, std::string>> extractStrings(
        const uint8_t* data, size_t dataSize,
        size_t minLength = 4
    ) const;

    std::string classifyString(const std::string& str) const;
    bool isFormatString(const std::string& str) const;
    bool isUrlString(const std::string& str) const;
    bool isHexString(const std::string& str) const;
};

} // namespace omnibyte::hydradis
