#pragma once
// UI/UILoader.h — load UIConfig from JSON.

#include "UIConfig.h"
#include <common/Serialization/JsonLoader.h>
#include <unordered_map>

namespace omnibyte::dumper::config {

namespace {
    const std::unordered_map<std::string, UIConfig::Verbosity> kVerbosityMap = {
        {"Quiet",   UIConfig::Verbosity::Quiet},
        {"Normal",  UIConfig::Verbosity::Normal},
        {"Verbose", UIConfig::Verbosity::Verbose},
        {"Debug",   UIConfig::Verbosity::Debug},
    };
} // anonymous namespace

inline UIConfig loadUIConfig(const std::string& path) {
    auto j = omnibyte::common::loadJsonFile(path);
    if (!j) return UIConfig::defaults();

    UIConfig cfg;
    cfg.theme     = omnibyte::common::getOr<std::string>(*j, "theme", cfg.theme);
    cfg.language  = omnibyte::common::getOr<std::string>(*j, "language", cfg.language);

    if (j->contains("verbosity") && j->at("verbosity").is_string()) {
        std::string v = j->at("verbosity").get<std::string>();
        auto it = kVerbosityMap.find(v);
        if (it != kVerbosityMap.end()) {
            cfg.verbosity = it->second;
        }
    }

    return cfg;
}

} // namespace omnibyte::dumper::config
