#pragma once
// ── Dumper1Engine.h — IL2CPP dumper adapted from il2cpp-dumper-rs ──
// Implements IDumperEngine. Orchestrates Analyzer + Resolver + Profiles.
// Detects binary format (ELF/PE/Mach-O/NSO/WASM), parses metadata with XOR
// decryption, finds CodeRegistration/MetadataRegistration, generates dump output.
#include "../../DumperCore/IDumperEngine.h"
#include "../../DumperCore/IEngineProfile.h"
#include "Analyzer/Dumper1Analyzer.h"
#include "Resolver/Dumper1Resolver.h"
#include "Profiles/V24Profile.h"
#include "Profiles/V27Profile.h"
#include "Profiles/V29Profile.h"
#include "Profiles/V31Profile.h"
#include <memory>
#include <string>
#include <vector>

namespace omnibyte::dumper::dumper1 {

class Dumper1Engine : public IDumperEngine {
public:
    EngineType type() const override { return EngineType::UnityIL2CPP; }
    std::string name() const override { return "Dumper1"; }

    DetectionResult detect(const AnalysisTarget& target) const override {
        DetectionResult r;

        if (!target.isFile()) return r;

        auto data = utils::readFileBytes(target.filePath);
        if (data.empty()) return r;

        // Check for IL2CPP metadata magic (0xAF1BBA00) or encrypted variant
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
                if (testMagic == 0xAF1BBA00) {
                    r.matched = true;
                    r.confidence = 0.8f;
                    r.detectedVersion = "encrypted";
                    return r;
                }
            }

            // Check for ELF binary containing IL2CPP
            if (magic == 0x464C457F) { // ELF magic
                // Scan for "global-metadata.dat" or "libil2cpp" strings
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
        // Version mapping from il2cpp-dumper-rs metadata parsing
        if (detectedVersion == "24") return std::make_shared<V24Profile>();
        if (detectedVersion == "27") return std::make_shared<V27Profile>();
        if (detectedVersion == "29") return std::make_shared<V29Profile>();
        if (detectedVersion == "31") return std::make_shared<V31Profile>();
        if (detectedVersion == "encrypted") return std::make_shared<V27Profile>(); // default
        if (detectedVersion == "elf_with_metadata") return std::make_shared<V27Profile>();
        return nullptr;
    }

    DumpData analyze(const AnalysisTarget& target,
                        const std::shared_ptr<IEngineProfile>& profile) override {
        return Dumper1Analyzer::analyze(target, profile);
    }

    DumpData resolveSymbols(const AnalysisTarget& target,
                               const std::shared_ptr<IEngineProfile>& profile) override {
        DumpData result;
        result.engineName = "Dumper1";
        result.detectedVersion = profile ? profile->version() : "unknown";

        if (!profile) {
            result.errorMessage = "No profile provided";
            return result;
        }

        // Get metadata counts from profile for registration search
        auto typeDefCount = static_cast<uint32_t>(profile->offsetOf("typeDefinitionCount"));
        auto methodDefCount = static_cast<uint32_t>(profile->offsetOf("methodDefinitionCount"));
        auto imageDefCount = static_cast<uint32_t>(profile->offsetOf("imageDefinitionCount"));

        auto regPair = Dumper1Resolver::resolve(target, profile,
                                                 typeDefCount, methodDefCount, imageDefCount);

        if (regPair.found()) {
            result.setMeta("codeRegistration", "0x" + toHex(regPair.codeRegistration));
            result.setMeta("metadataRegistration", "0x" + toHex(regPair.metadataRegistration));
            result.success = true;
        } else {
            result.errorMessage = "CodeRegistration/MetadataRegistration not found";
        }

        return result;
    }

    std::vector<std::string> supportedVersions() const override {
        return {"24", "27", "29", "31"};
    }

private:
    static std::string toHex(uint64_t val) {
        if (val == 0) return "0";
        char buf[32];
        snprintf(buf, sizeof(buf), "%llx", (unsigned long long)val);
        return std::string(buf);
    }
};

} // namespace omnibyte::dumper::dumper1
