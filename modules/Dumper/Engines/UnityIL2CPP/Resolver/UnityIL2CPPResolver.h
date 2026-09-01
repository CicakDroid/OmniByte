#pragma once
// UnityIL2CPP — runtime symbol resolver via xdl_sym.
// Resolves libil2cpp.so symbols and cross-references with metadata
// to get concrete method/type addresses.
#include "../../../DumperCore/IDumperEngine.h"
#include "../../../DumperCore/IEngineProfile.h"
#include "../../../DumperCore/SharedUtils/SharedUtils.h"
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

namespace omnibyte::dumper::unityil2cpp {

class UnityIL2CPPResolver {
public:
    static DumpResult resolveSymbols(const AnalysisTarget& target,
                                     const std::shared_ptr<IEngineProfile>& profile) {
        DumpResult result;
        result.engineName = "Unity IL2CPP";
        result.detectedVersion = profile ? profile->version() : "unknown";

        if (!profile) {
            result.errorMessage = "No profile provided";
            return result;
        }

        if (target.isFile()) {
            result.errorMessage = "Resolver requires live process target (use analyzer for file targets)";
            return result;
        }

        // Resolve IL2CPP API symbols via profile->symbolFor()
        resolveIL2CPPApiSymbols(target, profile, result);

        // Resolve metadata pointer via patternFor if available
        resolveMetadataPointer(target, profile, result);

        result.success = !result.metadata.empty();
        if (!result.success) {
            result.errorMessage = "No symbols resolved";
        }

        return result;
    }

private:
    // Core IL2CPP API symbols for type/method/field enumeration
    static constexpr const char* kClassSymbols[] = {
        "il2cpp_class_get_name",
        "il2cpp_class_get_namespace",
        "il2cpp_class_get_parent",
        "il2cpp_class_get_interfaces",
        "il2cpp_class_get_methods",
        "il2cpp_class_get_method_count",
        "il2cpp_class_get_fields",
        "il2cpp_class_get_field_count",
        "il2cpp_class_get_instance_size",
    };

    static constexpr const char* kMethodSymbols[] = {
        "il2cpp_method_get_name",
        "il2cpp_method_get_pointer",
        "il2cpp_method_get_return_type",
        "il2cpp_method_get_param_count",
    };

    static constexpr const char* kFieldSymbols[] = {
        "il2cpp_field_get_name",
        "il2cpp_field_get_type",
        "il2cpp_field_get_offset",
    };

    static constexpr const char* kImageSymbols[] = {
        "il2cpp_image_get_class",
        "il2cpp_image_get_class_count",
    };

    static void resolveIL2CPPApiSymbols(const AnalysisTarget& target,
                                         const std::shared_ptr<IEngineProfile>& profile,
                                         DumpResult& result) {
        // Resolve class enumeration symbols
        for (const char* sym : kClassSymbols) {
            tryResolveSymbol(profile, sym, result);
        }

        // Resolve method symbols
        for (const char* sym : kMethodSymbols) {
            tryResolveSymbol(profile, sym, result);
        }

        // Resolve field symbols
        for (const char* sym : kFieldSymbols) {
            tryResolveSymbol(profile, sym, result);
        }

        // Resolve image symbols
        for (const char* sym : kImageSymbols) {
            tryResolveSymbol(profile, sym, result);
        }
    }

    static void resolveMetadataPointer(const AnalysisTarget& target,
                                        const std::shared_ptr<IEngineProfile>& profile,
                                        DumpResult& result) {
        // Try to resolve GlobalMetadataPointer via AOB pattern
        auto pattern = profile->patternFor("GlobalMetadataPointer");
        if (pattern) {
            result.setMeta("GlobalMetadataPattern",
                pattern->bytePattern + " [" + pattern->mask + "]");
            result.setMeta("GlobalMetadataAddressOffset",
                std::to_string(pattern->addressOffsetFromMatch));
        }

        // Try to resolve MetadataCache pointer
        auto cachePattern = profile->patternFor("MetadataCachePointer");
        if (cachePattern) {
            result.setMeta("MetadataCachePattern",
                cachePattern->bytePattern + " [" + cachePattern->mask + "]");
        }
    }

    static void tryResolveSymbol(const std::shared_ptr<IEngineProfile>& profile,
                                  const std::string& key,
                                  DumpResult& result) {
        auto symName = profile->symbolFor(key);
        if (symName) {
            result.setMeta(key, *symName);
        }
    }
};

} // namespace omnibyte::dumper::unityil2cpp
