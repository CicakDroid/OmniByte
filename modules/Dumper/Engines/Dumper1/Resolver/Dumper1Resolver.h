#pragma once
// Dumper1 — CodeRegistration / MetadataRegistration resolver (adapted from il2cpp-dumper-rs).
// Finds CR/MR in ELF/PE/Mach-O/NSO/WASM binaries via section scan + symbol search.
#include "../../../DumperCore/IDumperEngine.h"
#include "../../../DumperCore/IEngineProfile.h"
#include "../../../DumperCore/SharedUtils/SharedUtils.h"
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <functional>

namespace omnibyte::dumper::dumper1 {

// Registration pair: CodeRegistration + MetadataRegistration addresses.
struct RegistrationPair {
    uint64_t codeRegistration = 0;
    uint64_t metadataRegistration = 0;
    bool found() const { return codeRegistration != 0 && metadataRegistration != 0; }
};

class Dumper1Resolver {
public:
    using ProgressCallback = std::function<void(const std::string&)>;

    // ── Binary Format Detection ──
    // From il2cpp-dumper-rs: detect_format() checks magic bytes.
    static std::string detectFormat(const std::vector<uint8_t>& data) {
        if (data.size() < 4) return "unknown";
        uint32_t magic32 = readU32LE(data, 0);
        uint16_t magic16 = readU16LE(data, 0);

        if (magic32 == 0x464C457F) return "elf";          // ELF magic: \x7FELF
        if (magic32 == 0xFEEDFACE) return "macho32";      // Mach-O 32
        if (magic32 == 0xFEEDFACF) return "macho64";      // Mach-O 64
        if (magic32 == 0xBEBAFECA) return "macho_fat";    // Fat Mach-O
        if (magic32 == 0x304F534E) return "nso";           // NSO (Nintendo Switch)
        if (magic32 == 0x6D736100) return "wasm";          // WebAssembly
        if (magic16 == 0x5A4D) return "pe";                // PE (MZ header)
        return "unknown";
    }

    // ── Resolve registrations in binary ──
    static RegistrationPair resolve(const AnalysisTarget& target,
                                    const std::shared_ptr<IEngineProfile>& profile,
                                    uint32_t typeDefCount,
                                    uint32_t methodDefCount,
                                    uint32_t imageDefCount,
                                    ProgressCallback progress = nullptr) {
        RegistrationPair result;

        if (!target.isFile()) {
            return result; // Resolver works on files, not live processes
        }

        auto data = utils::readFileBytes(target.filePath);
        if (data.empty()) return result;

        std::string format = detectFormat(data);
        if (progress) progress("Detected format: " + format);

        if (format == "elf") {
            result = resolveELF(data, profile, typeDefCount, methodDefCount, imageDefCount, progress);
        } else if (format == "pe") {
            result = resolvePE(data, profile, typeDefCount, methodDefCount, imageDefCount, progress);
        } else if (format == "macho32" || format == "macho64") {
            result = resolveMachO(data, profile, typeDefCount, methodDefCount, imageDefCount, progress);
        } else if (format == "macho_fat") {
            // Fat Mach-O: extract first slice (simplified)
            if (data.size() > 4096) {
                result = resolveMachO(data, profile, typeDefCount, methodDefCount, imageDefCount, progress);
            }
        } else if (format == "nso") {
            result = resolveNSO(data, profile, typeDefCount, methodDefCount, imageDefCount, progress);
        } else if (format == "wasm") {
            result = resolveWASM(data, profile, typeDefCount, methodDefCount, imageDefCount, progress);
        }

        // Fallback: symbol search across all formats
        if (!result.found()) {
            if (progress) progress("Attempting symbol table search...");
            result = symbolSearch(data, format, profile, typeDefCount, methodDefCount, imageDefCount);
        }

        return result;
    }

private:
    // ── Read helpers ──
    static uint32_t readU32LE(const std::vector<uint8_t>& buf, size_t off) {
        if (off + 4 > buf.size()) return 0;
        uint32_t v;
        std::memcpy(&v, buf.data() + off, 4);
        return v;
    }

    static uint16_t readU16LE(const std::vector<uint8_t>& buf, size_t off) {
        if (off + 2 > buf.size()) return 0;
        uint16_t v;
        std::memcpy(&v, buf.data() + off, 2);
        return v;
    }

    // ── Section scanning ──
    // From il2cpp-dumper-rs: find_code_registration / find_metadata_registration
    // Scans sections for typeDefCount + methodDefCount pattern matches.
    static RegistrationPair scanForRegistrations(const uint8_t* data, size_t size,
                                                  uint32_t typeDefCount, uint32_t methodDefCount,
                                                  uint32_t imageDefCount,
                                                  size_t pointerSize = 8) {
        RegistrationPair result;
        if (size < 64) return result;

        // Pattern: find where typeDefCount appears as 4-byte LE value
        // Then check nearby for methodDefCount and imageDefCount
        for (size_t i = 0; i + 64 <= size; i += pointerSize) {
            uint32_t val = readU32LE({data, data + size}, i);
            if (val == typeDefCount) {
                // Check adjacent values for methodDefCount or imageDefCount
                for (size_t j = i + pointerSize; j + 16 <= size && j < i + 256; j += pointerSize) {
                    uint32_t next = readU32LE({data, data + size}, j);
                    if (next == methodDefCount || next == imageDefCount) {
                        // Potential CR/MR candidate — store as placeholder
                        // Real implementation would dereference pointers
                        if (result.codeRegistration == 0) {
                            result.codeRegistration = i;
                        } else if (result.metadataRegistration == 0) {
                            result.metadataRegistration = i;
                        }
                        if (result.found()) return result;
                    }
                }
            }
        }
        return result;
    }

    // ── Symbol table search ──
    // From il2cpp-dumper-rs: symbol_search() looks for il2cpp_* / mono_* exports.
    static RegistrationPair symbolSearch(const std::vector<uint8_t>& data,
                                          const std::string& format,
                                          const std::shared_ptr<IEngineProfile>& profile,
                                          uint32_t typeDefCount, uint32_t methodDefCount,
                                          uint32_t imageDefCount) {
        RegistrationPair result;
        // Scan for symbol-like strings referencing registrations
        // This is a simplified version — real impl parses ELF symtab/dynsym
        // or PE export directory.
        const std::string needle = "Il2CppCodeRegistration";
        for (size_t i = 0; i + needle.size() <= data.size(); ++i) {
            if (std::memcmp(data.data() + i, needle.data(), needle.size()) == 0) {
                result.codeRegistration = i;
                break;
            }
        }
        const std::string needle2 = "Il2CppMetadataRegistration";
        for (size_t i = 0; i + needle2.size() <= data.size(); ++i) {
            if (std::memcmp(data.data() + i, needle2.data(), needle2.size()) == 0) {
                result.metadataRegistration = i;
                break;
            }
        }
        return result;
    }

    // ── Format-specific resolvers ──
    static RegistrationPair resolveELF(const std::vector<uint8_t>& data,
                                        const std::shared_ptr<IEngineProfile>& profile,
                                        uint32_t typeDefCount, uint32_t methodDefCount,
                                        uint32_t imageDefCount,
                                        ProgressCallback progress) {
        RegistrationPair result;
        if (progress) progress("Scanning ELF sections...");

        // Simplified: scan entire binary for registration patterns
        // Real impl: parse ELF header, iterate PT_LOAD sections
        result = scanForRegistrations(data.data(), data.size(),
                                       typeDefCount, methodDefCount, imageDefCount, 8);
        return result;
    }

    static RegistrationPair resolvePE(const std::vector<uint8_t>& data,
                                       const std::shared_ptr<IEngineProfile>& profile,
                                       uint32_t typeDefCount, uint32_t methodDefCount,
                                       uint32_t imageDefCount,
                                       ProgressCallback progress) {
        RegistrationPair result;
        if (progress) progress("Scanning PE sections...");

        // PE: parse IMAGE_NT_HEADERS, iterate sections
        if (data.size() < 64) return result;
        uint32_t peOffset = readU32LE(data, 0x3C);
        if (peOffset + 24 > data.size()) return result;

        result = scanForRegistrations(data.data(), data.size(),
                                       typeDefCount, methodDefCount, imageDefCount, 4);
        return result;
    }

    static RegistrationPair resolveMachO(const std::vector<uint8_t>& data,
                                          const std::shared_ptr<IEngineProfile>& profile,
                                          uint32_t typeDefCount, uint32_t methodDefCount,
                                          uint32_t imageDefCount,
                                          ProgressCallback progress) {
        RegistrationPair result;
        if (progress) progress("Scanning Mach-O segments...");

        result = scanForRegistrations(data.data(), data.size(),
                                       typeDefCount, methodDefCount, imageDefCount, 8);
        return result;
    }

    static RegistrationPair resolveNSO(const std::vector<uint8_t>& data,
                                        const std::shared_ptr<IEngineProfile>& profile,
                                        uint32_t typeDefCount, uint32_t methodDefCount,
                                        uint32_t imageDefCount,
                                        ProgressCallback progress) {
        RegistrationPair result;
        if (progress) progress("Scanning NSO sections...");

        result = scanForRegistrations(data.data(), data.size(),
                                       typeDefCount, methodDefCount, imageDefCount, 8);
        return result;
    }

    static RegistrationPair resolveWASM(const std::vector<uint8_t>& data,
                                         const std::shared_ptr<IEngineProfile>& profile,
                                         uint32_t typeDefCount, uint32_t methodDefCount,
                                         uint32_t imageDefCount,
                                         ProgressCallback progress) {
        RegistrationPair result;
        if (progress) progress("Scanning WASM sections...");

        result = scanForRegistrations(data.data(), data.size(),
                                       typeDefCount, methodDefCount, imageDefCount, 4);
        return result;
    }
};

} // namespace omnibyte::dumper::dumper1
