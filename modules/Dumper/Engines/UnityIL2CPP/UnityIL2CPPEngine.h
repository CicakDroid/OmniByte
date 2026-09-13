#pragma once
// ── UnityIL2CPPEngine.h — IL2CPP dumper engine (merged from Dumper1 + UnityIL2CPP) ──
// Implements IDumperEngine. Orchestrates Analyzer + Resolver + Profiles.
// Detects binary format, parses metadata with XOR decryption, finds
// CodeRegistration/MetadataRegistration, generates dump output.
#include "../../DumperCore/IDumperEngine.h"
#include "../../DumperCore/IEngineProfile.h"
#include "Analyzer/UnityIL2CPPAnalyzer.h"
#include "Resolver/UnityIL2CPPResolver.h"
#include "Profiles/V24Profile.h"
#include "Profiles/V24_1Profile.h"
#include "Profiles/V24_2Profile.h"
#include "Profiles/V27Profile.h"
#include "Profiles/V29Profile.h"
#include "Profiles/V31Profile.h"
#include <memory>
#include <string>
#include <vector>
#include <fstream>

namespace omnibyte::dumper::unityil2cpp {

class UnityIL2CPPEngine : public IDumperEngine {
public:
    EngineType type() const override { return EngineType::UnityIL2CPP; }
    std::string name() const override { return "Unity IL2CPP"; }

    DetectionResult detect(const AnalysisTarget& target) const override {
        DetectionResult r;

        if (!target.isFile()) return r;

        auto data = utils::readFileBytes(target.filePath);
        if (data.empty()) return r;

        // Check for IL2CPP metadata magic (0xFAB11BAF) or encrypted variant
        if (data.size() >= 8) {
            uint32_t magic;
            std::memcpy(&magic, data.data(), 4);

            // Direct magic match
            if (magic == 0xFAB11BAF) {
                uint32_t ver;
                std::memcpy(&ver, data.data() + 4, 4);
                r.matched = true;
                r.confidence = 1.0f;
                r.detectedVersion = std::to_string(ver);
                return r;
            }

            // Check for encrypted metadata (single-byte XOR)
            for (uint16_t k = 1; k < 256; ++k) {
                uint8_t test[4];
                for (int i = 0; i < 4; ++i) test[i] = data[i] ^ static_cast<uint8_t>(k);
                uint32_t testMagic;
                std::memcpy(&testMagic, test, 4);
                if (testMagic == 0xFAB11BAF) {
                    r.matched = true;
                    r.confidence = 0.8f;
                    r.detectedVersion = "encrypted";
                    return r;
                }
            }

            // Check for ELF binary containing IL2CPP
            if (magic == 0x464C457F) { // ELF magic
                const std::string metaName = "global-metadata.dat";
                for (size_t i = 0; i + metaName.size() <= data.size(); ++i) {
                    if (std::memcmp(data.data() + i, metaName.data(), metaName.size()) == 0) {
                        r.matched = true;
                        r.confidence = 0.7f;
                        r.detectedVersion = "elf_with_metadata";
                        return r;
                    }
                }
            }
        }

        return r;
    }

    std::shared_ptr<IEngineProfile> resolveProfile(
        const std::string& detectedVersion) const override {
        if (detectedVersion == "24") return std::make_shared<V24Profile>();
        if (detectedVersion == "24.1") return std::make_shared<V24_1Profile>();
        if (detectedVersion == "24.2") return std::make_shared<V24_2Profile>();
        if (detectedVersion == "27") return std::make_shared<V27Profile>();
        if (detectedVersion == "29") return std::make_shared<V29Profile>();
        if (detectedVersion == "31") return std::make_shared<V31Profile>();
        if (detectedVersion == "encrypted") return std::make_shared<V27Profile>();
        if (detectedVersion == "elf_with_metadata") return std::make_shared<V27Profile>();
        return nullptr;
    }

    DumpData analyze(const AnalysisTarget& target,
                        const std::shared_ptr<IEngineProfile>& profile) override {
        DumpData result = UnityIL2CPPAnalyzer::analyze(target, profile);

        if (result.success && !target.filePath.empty()) {
            generateOutput(result, target.filePath);
        }

        return result;
    }

    DumpData resolveSymbols(const AnalysisTarget& target,
                               const std::shared_ptr<IEngineProfile>& profile) override {
        DumpData result;
        result.engineName = "Unity IL2CPP";
        result.detectedVersion = profile ? profile->version() : "unknown";

        if (!profile) {
            result.errorMessage = "No profile provided";
            return result;
        }

        // Static binary analysis: find CodeRegistration/MetadataRegistration
        auto typeDefCount = static_cast<uint32_t>(profile->offsetOf("typeDefinitionCount"));
        auto methodDefCount = static_cast<uint32_t>(profile->offsetOf("methodDefinitionCount"));
        auto imageDefCount = static_cast<uint32_t>(profile->offsetOf("imageDefinitionCount"));

        auto regPair = UnityIL2CPPResolver::resolve(target, profile,
                                                     typeDefCount, methodDefCount, imageDefCount);

        if (regPair.found()) {
            result.setMeta("codeRegistration", toHex(regPair.codeRegistration));
            result.setMeta("metadataRegistration", toHex(regPair.metadataRegistration));
        }

        // Runtime symbol resolution (via xdl)
        auto runtimeResult = UnityIL2CPPResolver::resolveSymbols(target, profile);
        if (runtimeResult.success) {
            result.success = true;
        } else if (!regPair.found()) {
            result.errorMessage = "CodeRegistration/MetadataRegistration not found";
        } else {
            result.success = true;
        }

        return result;
    }

    std::vector<std::string> supportedVersions() const override {
        return {"24", "24.1", "24.2", "27", "29", "31"};
    }

private:
    static void generateOutput(const DumpData& data, const std::string& inputPath) {
        std::string basePath = inputPath;
        auto lastSlash = basePath.find_last_of("/\\");
        if (lastSlash != std::string::npos) {
            basePath = basePath.substr(0, lastSlash + 1);
        } else {
            basePath = "";
        }

        // CSharpWriter (existing)
        {
            std::ofstream ofs(basePath + "dump.cs");
            if (ofs.is_open()) {
                // Write type definitions
                for (const auto& t : data.typeTable) {
                    ofs << "public class " << t.name << " {" << std::endl;
                    for (const auto& m : data.methodTable) {
                        if (m.declaringTypeIndex == t.index) {
                            ofs << "    public void " << m.name << "() {}" << std::endl;
                        }
                    }
                    ofs << "}" << std::endl;
                }
            }
        }

        // StructGenerator
        {
            std::ofstream ofs(basePath + "struct_dump.cs");
            if (ofs.is_open()) {
                for (const auto& t : data.typeTable) {
                    ofs << "public struct " << t.name << " {" << std::endl;
                    for (const auto& f : data.fieldTable) {
                        if (f.declaringTypeIndex == t.index) {
                            ofs << "    public " << f.typeName << " " << f.name << ";" << std::endl;
                        }
                    }
                    ofs << "}" << std::endl;
                }
            }
        }

        // StaticFieldExporter
        {
            std::ofstream ofs(basePath + "static_fields.txt");
            if (ofs.is_open()) {
                ofs << "Static Field Offsets" << std::endl;
                ofs << "===================" << std::endl;
                for (const auto& f : data.fieldTable) {
                    if (f.isStatic) {
                        ofs << f.typeName << "::" << f.name << " @ 0x" << std::hex << f.offset << std::endl;
                    }
                }
            }
        }
    }

    static std::string toHex(uint64_t val) {
        if (val == 0) return "0x0";
        char buf[32];
        snprintf(buf, sizeof(buf), "0x%llx", (unsigned long long)val);
        return std::string(buf);
    }
};

} // namespace omnibyte::dumper::unityil2cpp
