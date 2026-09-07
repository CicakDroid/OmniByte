#pragma once
// UI/UILoader.h — load UIConfig from JSON.

#include "UIConfig.h"
#include <common/Serialization/JsonLoader.h>

namespace omnibyte::dumper::config {

inline UIConfig loadUIConfig(const std::string& path) {
    auto j = omnibyte::common::loadJsonFile(path);
    if (!j) return UIConfig::defaults();
    return UIConfig::fromJson(*j);
}

} // namespace omnibyte::dumper::config
