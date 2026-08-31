#pragma once
// UI/UIConfig.h — configuration for user interface display settings.

#include <string>
#include <unordered_map>

namespace omnibyte::dumper::config {

struct UIConfig {
    // Visual theme: "dark", "light", "auto", or any custom theme name.
    std::string theme = "dark";

    // UI language code (ISO 639-1): "en", "id", "ja", etc.
    std::string language = "en";

    // Output verbosity level.
    enum class Verbosity {
        Quiet,    // errors only
        Normal,   // info + errors
        Verbose,  // info + errors + warnings + debug summaries
        Debug     // full diagnostic output
    };

    Verbosity verbosity = Verbosity::Normal;

    static UIConfig defaults() { return {}; }
};

} // namespace omnibyte::dumper::config
