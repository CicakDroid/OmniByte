#pragma once
// ExportRegistry — Registry for export writers.
// Maps format names to IExporter instances and routes export requests.
#include "../IExporter/IExporter.h"
#include <algorithm>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace omnibyte::dumper::export_core {

class ExportRegistry {
public:
    static ExportRegistry& instance() {
        static ExportRegistry registry;
        return registry;
    }

    // Register an exporter for a format name.
    // If format already registered, replaces it.
    void registerExporter(const std::string& formatName,
                          std::shared_ptr<IExporter> exporter) {
        if (!exporter) return;
        exporters_[formatName] = std::move(exporter);
    }

    // Get exporter by format name.
    // Returns nullptr if not found.
    std::shared_ptr<IExporter> getExporter(const std::string& formatName) const {
        auto it = exporters_.find(formatName);
        if (it != exporters_.end()) return it->second;
        return nullptr;
    }

    // Check if a format is registered
    bool hasExporter(const std::string& formatName) const {
        return exporters_.find(formatName) != exporters_.end();
    }

    // Get all registered format names
    std::vector<std::string> getSupportedFormats() const {
        std::vector<std::string> formats;
        formats.reserve(exporters_.size());
        for (const auto& [name, _] : exporters_) {
            formats.push_back(name);
        }
        std::sort(formats.begin(), formats.end());
        return formats;
    }

    // Export using a specific format
    bool exportTo(const std::string& formatName,
                  const DumpResult& result,
                  const std::string& outputPath) const {
        auto exporter = getExporter(formatName);
        if (!exporter) return false;
        return exporter->exportToFile(result, outputPath);
    }

    // Export using a specific format to string
    std::string exportToString(const std::string& formatName,
                               const DumpResult& result) const {
        auto exporter = getExporter(formatName);
        if (!exporter) return "";
        return exporter->exportToString(result);
    }

    // Get description for all registered formats
    std::vector<std::pair<std::string, std::string>> listExporters() const {
        std::vector<std::pair<std::string, std::string>> list;
        for (const auto& [name, exporter] : exporters_) {
            list.emplace_back(name, exporter->description());
        }
        std::sort(list.begin(), list.end(),
                  [](const auto& a, const auto& b) { return a.first < b.first; });
        return list;
    }

private:
    ExportRegistry() = default;
    std::unordered_map<std::string, std::shared_ptr<IExporter>> exporters_;
};

} // namespace omnibyte::dumper::export_core
