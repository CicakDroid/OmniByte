#pragma once
// IExporter — Base interface for all dump export writers.
// Each writer (CSharp, JSON, Header, DummyDll) implements this interface
// to convert DumpResult into a specific output format.
#include "../../DumperCore/DumpResult.h"
#include <memory>
#include <string>
#include <vector>

namespace omnibyte::dumper::export_core {

class IExporter {
public:
    virtual ~IExporter() = default;

    // Format identifier (used for registry lookup)
    // e.g. "csharp", "json", "header", "dummydll"
    virtual std::string formatName() const = 0;

    // File extension including dot
    // e.g. ".cs", ".json", ".h"
    virtual std::string fileExtension() const = 0;

    // Export DumpResult to a file at outputPath
    // Returns true on success, false on failure
    virtual bool exportToFile(const DumpResult& result,
                              const std::string& outputPath) const = 0;

    // Export DumpResult to a string (for in-memory use or testing)
    virtual std::string exportToString(const DumpResult& result) const = 0;

    // Description of this exporter for help/listing
    virtual std::string description() const = 0;
};

} // namespace omnibyte::dumper::export_core
