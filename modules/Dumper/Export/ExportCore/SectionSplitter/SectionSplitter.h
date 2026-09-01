#pragma once
// SectionSplitter — Splits DumpResult into logical sections for organized output.
// Groups types by namespace/package, methods by declaring type, etc.
#include "../../../DumperCore/DumpResult.h"
#include <algorithm>
#include <map>
#include <string>
#include <vector>

namespace omnibyte::dumper::export_core {

struct TypeSection {
    std::string namespace_;         // namespace/package (e.g. "UE4", "UnityEngine")
    std::vector<TypeEntry> types;   // types in this section
};

struct MethodSection {
    std::string declaringType;
    std::vector<MethodEntry> methods;
};

class SectionSplitter {
public:
    // Split types by namespace/prefix convention.
    // UE4/UE5 types: prefix-based (e.g. "UObject" → "UE4")
    // IL2CPP types: dot-separated (e.g. "UnityEngine.GameObject" → "UnityEngine")
    static std::vector<TypeSection> splitByNamespace(const DumpResult& result) {
        std::map<std::string, TypeSection> sectionMap;

        for (const auto& type : result.typeTable) {
            std::string ns = extractNamespace(type.name, result.engineName);
            sectionMap[ns].namespace_ = ns;
            sectionMap[ns].types.push_back(type);
        }

        std::vector<TypeSection> sections;
        for (auto& [_, section] : sectionMap) {
            // Sort types within section by name
            std::sort(section.types.begin(), section.types.end(),
                      [](const TypeEntry& a, const TypeEntry& b) {
                          return a.name < b.name;
                      });
            sections.push_back(std::move(section));
        }

        return sections;
    }

    // Group methods by their declaring type
    static std::vector<MethodSection> groupMethodsByType(const DumpResult& result) {
        std::map<std::string, MethodSection> sectionMap;

        for (const auto& method : result.methodTable) {
            sectionMap[method.declaringType].declaringType = method.declaringType;
            sectionMap[method.declaringType].methods.push_back(method);
        }

        std::vector<MethodSection> sections;
        for (auto& [_, section] : sectionMap) {
            // Sort methods by index
            std::sort(section.methods.begin(), section.methods.end(),
                      [](const MethodEntry& a, const MethodEntry& b) {
                          return a.methodIndex < b.methodIndex;
                      });
            sections.push_back(std::move(section));
        }

        return sections;
    }

    // Get fields for a specific type
    static std::vector<FieldEntry> getFieldsForType(const DumpResult& result,
                                                     const std::string& typeName) {
        std::vector<FieldEntry> fields;
        for (const auto& field : result.fieldTable) {
            if (field.declaringType == typeName) {
                fields.push_back(field);
            }
        }
        // Sort by offset ascending
        std::sort(fields.begin(), fields.end(),
                  [](const FieldEntry& a, const FieldEntry& b) {
                      return a.offset < b.offset;
                  });
        return fields;
    }

    // Get methods for a specific type
    static std::vector<MethodEntry> getMethodsForType(const DumpResult& result,
                                                       const std::string& typeName) {
        std::vector<MethodEntry> methods;
        for (const auto& method : result.methodTable) {
            if (method.declaringType == typeName) {
                methods.push_back(method);
            }
        }
        std::sort(methods.begin(), methods.end(),
                  [](const MethodEntry& a, const MethodEntry& b) {
                      return a.methodIndex < b.methodIndex;
                  });
        return methods;
    }

private:
    // Extract namespace from type name based on engine conventions
    static std::string extractNamespace(const std::string& typeName,
                                         const std::string& engineName) {
        // IL2CPP/Godot: dot-separated namespaces
        auto dotPos = typeName.rfind('.');
        if (dotPos != std::string::npos) {
            return typeName.substr(0, dotPos);
        }

        // UE4/UE5: prefix-based (UObject, AActor, FVector, EEnum, etc.)
        if (engineName.find("Unreal") != std::string::npos ||
            engineName.find("UE") != std::string::npos) {
            return extractUENamespace(typeName);
        }

        // Default: use engine name as namespace
        return engineName;
    }

    // UE4/UE5 naming conventions
    static std::string extractUENamespace(const std::string& typeName) {
        if (typeName.empty()) return "Unknown";

        char prefix = typeName[0];
        switch (prefix) {
            case 'U': // UObject-derived
            case 'A': // AActor-derived
            case 'F': // FVector, FString, etc.
            case 'T': // TArray, TMap, etc.
            case 'E': // Enums
            case 'I': // Interfaces
                return "UE4";
            default:
                return "UE4";
        }
    }
};

} // namespace omnibyte::dumper::export_core
