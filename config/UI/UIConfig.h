#pragma once
// UI/UIConfig.h — configuration for user interface display settings.

#include <string>
#include <nlohmann/json.hpp>
#include <common/Serialization/JsonLoader.h>

namespace omnibyte::dumper::config {

struct UIConfig {
    std::string theme = "dark";
    std::string language = "id";

    enum class Verbosity {
        Quiet,    // errors only
        Normal,   // info + errors
        Verbose,  // info + errors + warnings + debug summaries
        Debug     // full diagnostic output
    };

    Verbosity verbosity = Verbosity::Normal;

    static UIConfig defaults() { return {}; }

    static UIConfig fromJson(const nlohmann::json& j) {
        UIConfig cfg;
        cfg.theme    = omnibyte::common::getOr<std::string>(j, "theme", cfg.theme);
        cfg.language = omnibyte::common::getOr<std::string>(j, "language", cfg.language);

        if (j.contains("verbosity") && j.at("verbosity").is_string()) {
            std::string v = j.at("verbosity").get<std::string>();
            if (v == "Quiet")        cfg.verbosity = Verbosity::Quiet;
            else if (v == "Normal")  cfg.verbosity = Verbosity::Normal;
            else if (v == "Verbose") cfg.verbosity = Verbosity::Verbose;
            else if (v == "Debug")   cfg.verbosity = Verbosity::Debug;
            // unknown string → keep default
        }

        return cfg;
    }

    nlohmann::json toJson() const {
        std::string vStr;
        switch (verbosity) {
            case Verbosity::Quiet:   vStr = "Quiet"; break;
            case Verbosity::Normal:  vStr = "Normal"; break;
            case Verbosity::Verbose: vStr = "Verbose"; break;
            case Verbosity::Debug:   vStr = "Debug"; break;
        }
        return {
            {"theme",     theme},
            {"language",  language},
            {"verbosity", vStr}
        };
    }
};

} // namespace omnibyte::dumper::config
