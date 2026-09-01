#pragma once
// JsonWriter — Generates structured JSON dump from DumpResult.
// Output format: JSON with types, methods, fields, strings, metadata.
#include "../../ExportCore/IExporter/IExporter.h"
#include "../../../DumperCore/DumpResult.h"
#include <fstream>
#include <sstream>
#include <iomanip>

namespace omnibyte::dumper::writers {

class JsonWriter : public export_core::IExporter {
public:
    std::string formatName() const override { return "json"; }
    std::string fileExtension() const override { return ".json"; }
    std::string description() const override {
        return "Structured JSON dump (machine-readable)";
    }

    bool exportToFile(const DumpResult& result,
                      const std::string& outputPath) const override {
        std::ofstream file(outputPath);
        if (!file.is_open()) return false;

        file << exportToString(result);
        return file.good();
    }

    std::string exportToString(const DumpResult& result) const override {
        std::ostringstream out;
        out << std::hex;

        // Root object
        out << "{\n";
        out << "  \"engine\": \"" << result.engineName << "\",\n";
        out << "  \"version\": \"" << result.detectedVersion << "\",\n";
        out << "  \"success\": " << (result.success ? "true" : "false") << ",\n";

        // Types
        out << "  \"types\": [\n";
        for (size_t i = 0; i < result.typeTable.size(); ++i) {
            const auto& type = result.typeTable[i];
            out << "    {\n";
            out << "      \"name\": \"" << escapeJson(type.name) << "\",\n";
            out << "      \"address\": \"0x" << type.address << "\",\n";
            out << "      \"size\": " << type.size << ",\n";
            out << "      \"typeId\": " << type.typeId << ",\n";
            out << "      \"parentType\": \"" << escapeJson(type.parentType) << "\",\n";
            out << "      \"interfaces\": [";
            for (size_t j = 0; j < type.interfaces.size(); ++j) {
                if (j > 0) out << ", ";
                out << "\"" << escapeJson(type.interfaces[j]) << "\"";
            }
            out << "]\n";

            // Methods for this type
            out << "      \"methods\": [\n";
            bool firstMethod = true;
            for (const auto& method : result.methodTable) {
                if (method.declaringType == type.name) {
                    if (!firstMethod) out << ",\n";
                    firstMethod = false;
                    out << "        {\n";
                    out << "          \"name\": \"" << escapeJson(method.name) << "\",\n";
                    out << "          \"address\": \"0x" << method.address << "\",\n";
                    out << "          \"methodIndex\": " << method.methodIndex << ",\n";
                    out << "          \"isVirtual\": " << (method.isVirtual ? "true" : "false") << ",\n";
                    out << "          \"isStatic\": " << (method.isStatic ? "true" : "false") << "\n";
                    out << "        }";
                }
            }
            out << "\n      ],\n";

            // Fields for this type
            out << "      \"fields\": [\n";
            bool firstField = true;
            for (const auto& field : result.fieldTable) {
                if (field.declaringType == type.name) {
                    if (!firstField) out << ",\n";
                    firstField = false;
                    out << "        {\n";
                    out << "          \"name\": \"" << escapeJson(field.name) << "\",\n";
                    out << "          \"offset\": " << field.offset << ",\n";
                    out << "          \"typeName\": \"" << escapeJson(field.typeName) << "\",\n";
                    out << "          \"fieldSize\": " << field.fieldSize << "\n";
                    out << "        }";
                }
            }
            out << "\n      ]\n";
            out << "    }" << (i < result.typeTable.size() - 1 ? "," : "") << "\n";
        }
        out << "  ],\n";

        // Strings
        out << "  \"strings\": [\n";
        for (size_t i = 0; i < result.stringTable.size(); ++i) {
            const auto& str = result.stringTable[i];
            out << "    {\n";
            out << "      \"value\": \"" << escapeJson(str.value) << "\",\n";
            out << "      \"address\": \"0x" << str.address << "\"\n";
            out << "    }" << (i < result.stringTable.size() - 1 ? "," : "") << "\n";
        }
        out << "  ],\n";

        // Metadata
        out << "  \"metadata\": {\n";
        bool firstMeta = true;
        for (const auto& [key, value] : result.metadata) {
            if (!firstMeta) out << ",\n";
            firstMeta = false;
            out << "    \"" << escapeJson(key) << "\": \"" << escapeJson(value) << "\"";
        }
        out << "\n  }\n";
        out << "}\n";

        return out.str();
    }

private:
    static std::string escapeJson(const std::string& s) {
        std::string result;
        result.reserve(s.size());
        for (char c : s) {
            switch (c) {
                case '"':  result += "\\\""; break;
                case '\\': result += "\\\\"; break;
                case '\n': result += "\\n"; break;
                case '\r': result += "\\r"; break;
                case '\t': result += "\\t"; break;
                default: result += c;
            }
        }
        return result;
    }
};

} // namespace omnibyte::dumper::writers
