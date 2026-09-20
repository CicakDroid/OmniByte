#pragma once
// ExportCore — Unified export orchestrator.
// Combines ExportRegistry (writer lookup), SectionSplitter (DumpData organization),
// and IExporter (output generation) into a single facade for the Dumper pipeline.
#include "ExportRegistry/ExportRegistry.h"
#include "SectionSplitter/SectionSplitter.h"
#include "../../DumperCore/DumpResult.h"
#include <string>
#include <vector>

namespace omnibyte::dumper::export_core {

struct ExportResult {
    bool success = false;
    std::string outputPath;
    std::string formatName;
    std::string errorMessage;
};

class ExportCore {
public:
    static ExportCore& instance() {
        static ExportCore core;
        return core;
    }

    void initialize() {
        ExportRegistry::instance().registerDefaults();
    }

    ExportResult exportToFile(const DumpData& data,
                              const std::string& formatName,
                              const std::string& outputPath) const {
        ExportResult result;
        result.formatName = formatName;
        result.outputPath = outputPath;

        auto& registry = ExportRegistry::instance();
        auto exporter = registry.getExporter(formatName);
        if (!exporter) {
            result.errorMessage = "Unknown format: " + formatName;
            return result;
        }

        result.success = exporter->exportToFile(data, outputPath);
        if (!result.success) {
            result.errorMessage = "Export failed for format: " + formatName;
        }
        return result;
    }

    std::string exportToString(const DumpData& data,
                               const std::string& formatName) const {
        auto exporter = ExportRegistry::instance().getExporter(formatName);
        if (!exporter) return "";
        return exporter->exportToString(data);
    }

    std::vector<ExportResult> exportAll(const DumpData& data,
                                        const std::string& outputDir,
                                        const std::string& baseName) const {
        std::vector<ExportResult> results;
        for (const auto& format : ExportRegistry::instance().getSupportedFormats()) {
            auto exporter = ExportRegistry::instance().getExporter(format);
            if (!exporter) continue;
            std::string path = outputDir + "/" + baseName + exporter->fileExtension();
            results.push_back(exportToFile(data, format, path));
        }
        return results;
    }

    std::vector<std::string> supportedFormats() const {
        return ExportRegistry::instance().getSupportedFormats();
    }
};

} // namespace omnibyte::dumper::export_core
