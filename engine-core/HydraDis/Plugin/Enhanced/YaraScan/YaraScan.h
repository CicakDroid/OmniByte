#pragma once
// YaraScan — YARA rule scanning engine.
// Optional plugin: requires libyara (https://github.com/VirusTotal/libyara).
// License: GPL-3.0 (libyara)

#include <cstdint>
#include <string>
#include <vector>

namespace omnibyte::deob {

struct YaraHit {
    uintptr_t offset = 0;
    std::string ruleName;
    std::string meta;
    double confidence = 0.0;
};

class YaraScanEngine {
public:
    YaraScanEngine() = default;
    ~YaraScanEngine();

    bool initialize();
    bool addRules(const std::string& rulesText);
    std::vector<YaraHit> scanRegion(const uint8_t* data, size_t size) const;
    std::vector<YaraHit> scanFile(const std::string& filePath) const;
    bool isInitialized() const;

private:
    struct Impl;
    Impl* impl_ = nullptr;
};

}  // namespace omnibyte::deob
