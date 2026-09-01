#pragma once
// KeyDot — Godot encryption key extractor.
// Source: https://github.com/Titoot/KeyDot (MIT)

#include <cstdint>
#include <string>
#include <vector>

namespace omnibyte::dumper {

/// Extracted key information.
struct ExtractedKey {
    std::string name;
    std::vector<uint8_t> value;
    std::string type;  // "encryption", "signing", "custom"
    uintptr_t offset = 0;
    double confidence = 0.0;
};

/// Base class for key extraction.
class IKeyExtractor {
public:
    virtual ~IKeyExtractor() = default;
    virtual std::vector<ExtractedKey> extractKeys(const std::string& filePath) const = 0;
    virtual std::string getEngineName() const = 0;
};

/// Godot engine encryption key extractor.
/// Scans Godot executables for encryption key patterns.
class GodotKeyExtractor : public IKeyExtractor {
public:
    GodotKeyExtractor() = default;
    ~GodotKeyExtractor() override = default;

    /// Extract encryption keys from a Godot executable.
    std::vector<ExtractedKey> extractKeys(const std::string& filePath) const override;

    std::string getEngineName() const override { return "Godot"; }

    /// Scan for Godot key structure patterns.
    std::vector<ExtractedKey> scanForKeyPatterns(const uint8_t* data, size_t size) const;

private:
    /// Known Godot key structure signatures.
    static const std::vector<std::vector<uint8_t>> keyPatterns_;

    /// Validate extracted key (check for reasonable entropy).
    bool validateKey(const std::vector<uint8_t>& key) const;
};

}  // namespace omnibyte::dumper
