#pragma once
// Dumper1 — CodeRegistration / MetadataRegistration resolver (adapted from il2cpp-dumper-rs).
// Finds CR/MR in ELF/PE/Mach-O/NSO/WASM binaries via proper format parsing.
// ELF: parses section headers (SHT_SYMTAB/SHT_DYNSYM) + string table.
// PE: walks export directory + section table.
// Mach-O: walks nlist symbol table + string table.
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
#include <algorithm>

namespace omnibyte::dumper::dumper1 {

struct RegistrationPair {
    uint64_t codeRegistration = 0;
    uint64_t metadataRegistration = 0;
    bool found() const { return codeRegistration != 0 && metadataRegistration != 0; }
};

class Dumper1Resolver {
public:
    using ProgressCallback = std::function<void(const std::string&)>;

    // ── Binary Format Detection ──
    static std::string detectFormat(const std::vector<uint8_t>& data) {
        if (data.size() < 4) return "unknown";
        uint32_t magic32 = readU32LE(data, 0);
        uint16_t magic16 = readU16LE(data, 0);

        if (magic32 == 0x464C457F) return "elf";
        if (magic32 == 0xFEEDFACE) return "macho32";
        if (magic32 == 0xFEEDFACF) return "macho64";
        if (magic32 == 0xBEBAFECA) return "macho_fat";
        if (magic32 == 0x304F534E) return "nso";
        if (magic32 == 0x6D736100) return "wasm";
        if (magic16 == 0x5A4D) return "pe";
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

        if (!target.isFile()) return result;

        auto data = utils::readFileBytes(target.filePath);
        if (data.empty()) return result;

        std::string format = detectFormat(data);
        if (progress) progress("Detected format: " + format);

        if (format == "elf") {
            result = resolveELF(data, typeDefCount, methodDefCount, imageDefCount, progress);
        } else if (format == "pe") {
            result = resolvePE(data, typeDefCount, methodDefCount, imageDefCount, progress);
        } else if (format == "macho32" || format == "macho64") {
            result = resolveMachO(data, format == "macho64",
                                  typeDefCount, methodDefCount, imageDefCount, progress);
        } else if (format == "macho_fat") {
            // Fat Mach-O: try to extract first slice.
            result = resolveFatMachO(data, typeDefCount, methodDefCount, imageDefCount, progress);
        } else if (format == "nso") {
            result = resolveNSO(data, typeDefCount, methodDefCount, imageDefCount, progress);
        } else if (format == "wasm") {
            result = resolveWASM(data, typeDefCount, methodDefCount, imageDefCount, progress);
        }

        // Fallback: scan for symbol-like strings if format-specific failed.
        if (!result.found()) {
            if (progress) progress("Attempting string pattern fallback...");
            result = stringPatternSearch(data, typeDefCount, methodDefCount, imageDefCount);
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

    static uint64_t readU64LE(const std::vector<uint8_t>& buf, size_t off) {
        if (off + 8 > buf.size()) return 0;
        uint64_t v;
        std::memcpy(&v, buf.data() + off, 8);
        return v;
    }

    static std::string readString(const std::vector<uint8_t>& buf, size_t off) {
        if (off >= buf.size()) return "";
        size_t end = off;
        while (end < buf.size() && buf[end] != 0) ++end;
        return std::string(reinterpret_cast<const char*>(buf.data() + off), end - off);
    }

    // ── Scan for registration counts in a data region ──
    static RegistrationPair scanForRegistrations(const uint8_t* data, size_t size,
                                                  uint32_t typeDefCount, uint32_t methodDefCount,
                                                  uint32_t imageDefCount,
                                                  size_t pointerSize = 8) {
        RegistrationPair result;
        if (size < 64) return result;

        for (size_t i = 0; i + 64 <= size; i += pointerSize) {
            uint32_t val = readU32LE({data, data + size}, i);
            if (val == typeDefCount) {
                for (size_t j = i + pointerSize; j + 16 <= size && j < i + 256; j += pointerSize) {
                    uint32_t next = readU32LE({data, data + size}, j);
                    if (next == methodDefCount || next == imageDefCount) {
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

    // ══════════════════════════════════════════════════════════════════════
    // ELF — Proper section header + symbol table parsing
    // ══════════════════════════════════════════════════════════════════════

    // ELF header constants.
    static constexpr uint16_t ELF_SHT_SYMTAB  = 2;
    static constexpr uint16_t ELF_SHT_STRTAB  = 3;
    static constexpr uint16_t ELF_SHT_DYNSYM  = 11;

    struct ElfSection {
        uint32_t name = 0;
        uint32_t type = 0;
        uint64_t offset = 0;
        uint64_t size = 0;
        uint32_t link = 0;
    };

    struct ElfSymbol {
        uint32_t nameOffset = 0;
        uint64_t value = 0;
        uint64_t size = 0;
        uint8_t  info = 0;
        uint16_t shndx = 0;
    };

    static RegistrationPair resolveELF(const std::vector<uint8_t>& data,
                                        uint32_t typeDefCount, uint32_t methodDefCount,
                                        uint32_t imageDefCount,
                                        ProgressCallback progress) {
        RegistrationPair result;
        if (data.size() < 64) return result;

        // Parse ELF header.
        uint8_t ei_class = data[4]; // 1 = 32-bit, 2 = 64-bit
        bool is64 = (ei_class == 2);
        size_t pointerSize = is64 ? 8 : 4;

        if (progress) progress("ELF: parsing section headers...");

        // Read section header table info.
        size_t shoff, shentsize, shnum, shstrndx;
        if (is64) {
            if (data.size() < 64) return result;
            shoff = readU64LE(data, 40);
            shentsize = readU16LE(data, 58);
            shnum = readU16LE(data, 60);
            shstrndx = readU16LE(data, 62);
        } else {
            if (data.size() < 52) return result;
            shoff = readU32LE(data, 32);
            shentsize = readU16LE(data, 46);
            shnum = readU16LE(data, 48);
            shstrndx = readU16LE(data, 50);
        }

        if (shoff == 0 || shnum == 0 || shentsize == 0) {
            // No section headers — fall back to segment scan.
            if (progress) progress("ELF: no section headers, scanning PT_LOAD segments...");
            return resolveELFSegmentScan(data, is64, typeDefCount, methodDefCount, imageDefCount);
        }

        // Parse all section headers.
        std::vector<ElfSection> sections(shnum);
        for (uint32_t i = 0; i < shnum; ++i) {
            size_t shdrOff = shoff + static_cast<size_t>(i) * shentsize;
            if (shdrOff + shentsize > data.size()) break;

            if (is64) {
                sections[i].name   = readU32LE(data, shdrOff + 0);
                sections[i].type   = readU32LE(data, shdrOff + 4);
                sections[i].offset = readU64LE(data, shdrOff + 24);
                sections[i].size   = readU64LE(data, shdrOff + 32);
                sections[i].link   = readU32LE(data, shdrOff + 40);
            } else {
                sections[i].name   = readU32LE(data, shdrOff + 0);
                sections[i].type   = readU32LE(data, shdrOff + 4);
                sections[i].offset = readU32LE(data, shdrOff + 16);
                sections[i].size   = readU32LE(data, shdrOff + 20);
                sections[i].link   = readU32LE(data, shdrOff + 24);
            }
        }

        // Read section name string table.
        auto getSectionName = [&](const ElfSection& sec) -> std::string {
            if (shstrndx >= sections.size()) return "";
            const auto& strtab = sections[shstrndx];
            return readString(data, strtab.offset + sec.name);
        };

        // Try to find symbol tables (SHT_SYMTAB or SHT_DYNSYM).
        for (uint32_t si = 0; si < shnum; ++si) {
            if (sections[si].type != ELF_SHT_SYMTAB && sections[si].type != ELF_SHT_DYNSYM) continue;
            if (sections[si].link >= sections.size()) continue;

            const auto& strtabSec = sections[sections[si].link];
            size_t entrySize = (is64) ? 24 : 16;
            uint32_t symCount = static_cast<uint32_t>(sections[si].size / entrySize);

            if (progress) {
                std::string tblType = (sections[si].type == ELF_SHT_SYMTAB) ? "symtab" : "dynsym";
                progress("ELF: scanning " + tblType + " (" + std::to_string(symCount) + " symbols)...");
            }

            for (uint32_t symIdx = 1; symIdx < symCount; ++symIdx) {
                size_t symOff = sections[si].offset + static_cast<size_t>(symIdx) * entrySize;
                if (symOff + entrySize > data.size()) break;

                ElfSymbol sym;
                if (is64) {
                    sym.nameOffset = readU32LE(data, symOff + 0);
                    sym.info       = data[symOff + 4];
                    sym.value      = readU64LE(data, symOff + 8);
                    sym.size       = readU64LE(data, symOff + 16);
                    sym.shndx      = readU16LE(data, symOff + 6);
                } else {
                    sym.nameOffset = readU32LE(data, symOff + 0);
                    sym.value      = readU32LE(data, symOff + 4);
                    sym.size       = readU32LE(data, symOff + 8);
                    sym.info       = data[symOff + 12];
                    sym.shndx      = readU16LE(data, symOff + 14);
                }

                if (sym.nameOffset == 0 || sym.shndx == 0) continue;

                std::string name = readString(data, strtabSec.offset + sym.nameOffset);
                if (name.empty()) continue;

                // Match il2cpp registration symbol names.
                if (name.find("Il2CppCodeRegistration") != std::string::npos ||
                    name.find("g_CodeRegistration") != std::string::npos) {
                    result.codeRegistration = sym.value;
                } else if (name.find("Il2CppMetadataRegistration") != std::string::npos ||
                           name.find("g_MetadataRegistration") != std::string::npos) {
                    result.metadataRegistration = sym.value;
                }

                if (result.found()) return result;
            }
        }

        // Symbol table search didn't find them — fall back to section scan.
        if (!result.found()) {
            if (progress) progress("ELF: symbol search inconclusive, scanning data sections...");
            for (uint32_t si = 0; si < shnum; ++si) {
                if (sections[si].type == 0 || sections[si].size == 0) continue;
                // Skip symbol/string tables and empty sections.
                if (sections[si].type == ELF_SHT_SYMTAB || sections[si].type == ELF_SHT_DYNSYM ||
                    sections[si].type == ELF_SHT_STRTAB) continue;

                size_t secEnd = sections[si].offset + sections[si].size;
                if (secEnd > data.size()) continue;

                auto sub = scanForRegistrations(data.data() + sections[si].offset,
                                                 sections[si].size,
                                                 typeDefCount, methodDefCount, imageDefCount,
                                                 pointerSize);
                if (sub.found()) {
                    result.codeRegistration = sub.codeRegistration;
                    result.metadataRegistration = sub.metadataRegistration;
                    return result;
                }
            }
        }

        return result;
    }

    // ELF fallback: scan PT_LOAD segments when section headers are stripped.
    static RegistrationPair resolveELFSegmentScan(const std::vector<uint8_t>& data,
                                                    bool is64,
                                                    uint32_t typeDefCount, uint32_t methodDefCount,
                                                    uint32_t imageDefCount) {
        RegistrationPair result;
        // Read program header table.
        size_t phoff;
        uint32_t phnum, phentsize;
        if (is64) {
            phoff = readU64LE(data, 32);
            phentsize = readU16LE(data, 54);
            phnum = readU16LE(data, 56);
        } else {
            phoff = readU32LE(data, 28);
            phentsize = readU16LE(data, 42);
            phnum = readU16LE(data, 44);
        }

        if (phoff == 0 || phnum == 0 || phentsize == 0) return result;

        for (uint32_t i = 0; i < phnum; ++i) {
            size_t phdrOff = phoff + static_cast<size_t>(i) * phentsize;
            if (phdrOff + phentsize > data.size()) break;

            uint32_t p_type;
            if (is64) {
                p_type = readU32LE(data, phdrOff);
            } else {
                p_type = readU32LE(data, phdrOff);
            }

            // PT_LOAD = 1
            if (p_type != 1) continue;

            size_t segOffset, segSize;
            if (is64) {
                segOffset = readU64LE(data, phdrOff + 8);
                segSize = readU64LE(data, phdrOff + 32);
            } else {
                segOffset = readU32LE(data, phdrOff + 4);
                segSize = readU32LE(data, phdrOff + 16);
            }

            if (segOffset + segSize > data.size()) segSize = data.size() - segOffset;
            if (segSize < 64) continue;

            auto sub = scanForRegistrations(data.data() + segOffset, segSize,
                                             typeDefCount, methodDefCount, imageDefCount,
                                             is64 ? 8 : 4);
            if (sub.found()) {
                result.codeRegistration = sub.codeRegistration;
                result.metadataRegistration = sub.metadataRegistration;
                return result;
            }
        }

        return result;
    }

    // ══════════════════════════════════════════════════════════════════════
    // PE — Export directory + section table parsing
    // ══════════════════════════════════════════════════════════════════════

    struct PESection {
        char     name[9] = {};
        uint32_t virtualSize = 0;
        uint32_t virtualAddr = 0;
        uint32_t rawOffset = 0;
        uint32_t rawSize = 0;
    };

    static RegistrationPair resolvePE(const std::vector<uint8_t>& data,
                                       uint32_t typeDefCount, uint32_t methodDefCount,
                                       uint32_t imageDefCount,
                                       ProgressCallback progress) {
        RegistrationPair result;
        if (data.size() < 64) return result;

        uint32_t peOffset = readU32LE(data, 0x3C);
        if (peOffset + 24 > data.size()) return result;
        if (readU32LE(data, peOffset) != 0x00004550) return result; // "PE\0\0"

        uint16_t numSections = readU16LE(data, peOffset + 6);
        uint16_t optHeaderSize = readU16LE(data, peOffset + 20);
        uint16_t optMagic = readU16LE(data, peOffset + 24);
        bool isPE32Plus = (optMagic == 0x20B);
        size_t pointerSize = isPE32Plus ? 8 : 4;

        size_t sectionTableOff = peOffset + 24 + optHeaderSize;

        // Read optional header: find NumberOfRvaAndSizes and data directory.
        size_t exportDirRVA = 0, exportDirSize = 0;
        if (isPE32Plus) {
            // PE32+: skip 112 bytes of standard fields, then data directories start.
            size_t ddOff = peOffset + 24 + 112;
            if (ddOff + 8 <= data.size()) {
                exportDirRVA = readU32LE(data, ddOff);
                exportDirSize = readU32LE(data, ddOff + 4);
            }
        } else {
            // PE32: skip 96 bytes of standard fields.
            size_t ddOff = peOffset + 24 + 96;
            if (ddOff + 8 <= data.size()) {
                exportDirRVA = readU32LE(data, ddOff);
                exportDirSize = readU32LE(data, ddOff + 4);
            }
        }

        // Parse section table.
        std::vector<PESection> sections(numSections);
        for (uint16_t i = 0; i < numSections; ++i) {
            size_t secOff = sectionTableOff + static_cast<size_t>(i) * 40;
            if (secOff + 40 > data.size()) break;

            std::memcpy(sections[i].name, data.data() + secOff, 8);
            sections[i].virtualSize = readU32LE(data, secOff + 8);
            sections[i].virtualAddr = readU32LE(data, secOff + 12);
            sections[i].rawSize     = readU32LE(data, secOff + 16);
            sections[i].rawOffset   = readU32LE(data, secOff + 20);
        }

        // Helper: convert RVA to file offset.
        auto rvaToFileOffset = [&](uint32_t rva) -> size_t {
            for (const auto& sec : sections) {
                if (rva >= sec.virtualAddr && rva < sec.virtualAddr + sec.rawSize) {
                    return sec.rawOffset + (rva - sec.virtualAddr);
                }
            }
            return 0;
        };

        // Parse export directory for symbol names.
        if (progress) progress("PE: parsing export directory...");

        if (exportDirRVA > 0 && exportDirSize >= 40) {
            size_t exportOff = rvaToFileOffset(exportDirRVA);
            if (exportOff + 40 <= data.size()) {
                uint32_t numFunctions = readU32LE(data, exportOff + 20);
                uint32_t numNames     = readU32LE(data, exportOff + 24);
                uint32_t funcRVA     = readU32LE(data, exportOff + 28);
                uint32_t nameRVA     = readU32LE(data, exportOff + 32);
                uint32_t ordinalRVA  = readU32LE(data, exportOff + 36);

                // Read name pointers and ordinals.
                size_t namePtrOff = rvaToFileOffset(nameRVA);
                size_t ordinalTblOff = rvaToFileOffset(ordinalRVA);
                size_t funcTblOff = rvaToFileOffset(funcRVA);

                if (namePtrOff > 0 && ordinalTblOff > 0) {
                    for (uint32_t ni = 0; ni < numNames; ++ni) {
                        size_t npEntry = namePtrOff + static_cast<size_t>(ni) * 4;
                        size_t orEntry = ordinalTblOff + static_cast<size_t>(ni) * 2;
                        if (npEntry + 4 > data.size() || orEntry + 2 > data.size()) break;

                        uint32_t nRVA = readU32LE(data, npEntry);
                        size_t nameOff = rvaToFileOffset(nRVA);
                        std::string name = readString(data, nameOff);

                        uint16_t ordinal = readU16LE(data, orEntry);
                        // Get the function address from the function table.
                        size_t funcEntry = funcTblOff + static_cast<size_t>(ordinal) * 4;
                        uint64_t funcAddr = 0;
                        if (funcEntry + 4 <= data.size()) {
                            funcAddr = readU32LE(data, funcEntry);
                        }

                        if (name.find("Il2CppCodeRegistration") != std::string::npos ||
                            name.find("g_CodeRegistration") != std::string::npos) {
                            result.codeRegistration = funcAddr;
                        } else if (name.find("Il2CppMetadataRegistration") != std::string::npos ||
                                   name.find("g_MetadataRegistration") != std::string::npos) {
                            result.metadataRegistration = funcAddr;
                        }

                        if (result.found()) return result;
                    }
                }
            }
        }

        // Fallback: scan each section for registration patterns.
        if (progress) progress("PE: scanning sections for registration patterns...");
        for (const auto& sec : sections) {
            if (sec.rawOffset == 0 || sec.rawSize == 0) continue;
            size_t secEnd = sec.rawOffset + sec.rawSize;
            if (secEnd > data.size()) continue;

            auto sub = scanForRegistrations(data.data() + sec.rawOffset, sec.rawSize,
                                             typeDefCount, methodDefCount, imageDefCount,
                                             pointerSize);
            if (sub.found()) {
                result.codeRegistration = sub.codeRegistration;
                result.metadataRegistration = sub.metadataRegistration;
                return result;
            }
        }

        return result;
    }

    // ══════════════════════════════════════════════════════════════════════
    // Mach-O — nlist symbol table + LC_SYMTAB parsing
    // ══════════════════════════════════════════════════════════════════════

    struct MachOSegment {
        char name[17] = {};
        uint64_t vmAddr = 0;
        uint64_t vmSize = 0;
        uint64_t fileOffset = 0;
        uint64_t fileSize = 0;
    };

    static RegistrationPair resolveMachO(const std::vector<uint8_t>& data,
                                           bool is64,
                                           uint32_t typeDefCount, uint32_t methodDefCount,
                                           uint32_t imageDefCount,
                                           ProgressCallback progress) {
        RegistrationPair result;
        if (data.size() < 32) return result;

        uint32_t ncmds = readU32LE(data, 16);
        uint32_t sizeofcmds = readU32LE(data, 20);

        // Walk load commands looking for LC_SYMTAB (0x02) and segments.
        std::vector<MachOSegment> segments;
        size_t cmdOff = is64 ? 32 : 28;
        size_t symtabCmdOff = 0;

        for (uint32_t ci = 0; ci < ncmds && cmdOff + 8 <= data.size(); ++ci) {
            uint32_t cmd = readU32LE(data, cmdOff);
            uint32_t cmdsize = readU32LE(data, cmdOff + 4);
            if (cmdsize == 0 || cmdOff + cmdsize > data.size()) break;

            if (cmd == 0x02) { // LC_SYMTAB
                symtabCmdOff = cmdOff;
            }

            // LC_SEGMENT / LC_SEGMENT_64
            if (cmd == 0x01 || cmd == 0x19) {
                MachOSegment seg;
                size_t nameLen = (cmd == 0x19) ? 16 : 16;
                std::memcpy(seg.name, data.data() + cmdOff + 8, nameLen);
                seg.name[nameLen] = '\0';

                if (cmd == 0x19) { // LC_SEGMENT_64
                    seg.vmAddr     = readU64LE(data, cmdOff + 24);
                    seg.vmSize     = readU64LE(data, cmdOff + 32);
                    seg.fileOffset = readU64LE(data, cmdOff + 40);
                    seg.fileSize   = readU64LE(data, cmdOff + 48);
                } else { // LC_SEGMENT
                    seg.vmAddr     = readU32LE(data, cmdOff + 24);
                    seg.vmSize     = readU32LE(data, cmdOff + 28);
                    seg.fileOffset = readU32LE(data, cmdOff + 32);
                    seg.fileSize   = readU32LE(data, cmdOff + 36);
                }

                segments.push_back(seg);
            }

            cmdOff += cmdsize;
        }

        // Parse LC_SYMTAB for symbol table info.
        if (symtabCmdOff > 0) {
            if (progress) progress("Mach-O: parsing LC_SYMTAB...");

            uint32_t symOff = readU32LE(data, symtabCmdOff + 8);
            uint32_t numSyms = readU32LE(data, symtabCmdOff + 12);
            uint32_t strOff = readU32LE(data, symtabCmdOff + 16);

            size_t nlistSize = is64 ? 16 : 12;

            for (uint32_t si = 0; si < numSyms; ++si) {
                size_t entryOff = symOff + static_cast<size_t>(si) * nlistSize;
                if (entryOff + nlistSize > data.size()) break;

                uint32_t strIdx;
                uint64_t value;
                if (is64) {
                    strIdx = readU32LE(data, entryOff);
                    value = readU64LE(data, entryOff + 8);
                } else {
                    strIdx = readU32LE(data, entryOff);
                    value = readU32LE(data, entryOff + 8);
                }

                std::string name = readString(data, strOff + strIdx);

                if (name.find("Il2CppCodeRegistration") != std::string::npos ||
                    name.find("_g_CodeRegistration") != std::string::npos) {
                    result.codeRegistration = value;
                } else if (name.find("Il2CppMetadataRegistration") != std::string::npos ||
                           name.find("_g_MetadataRegistration") != std::string::npos) {
                    result.metadataRegistration = value;
                }

                if (result.found()) return result;
            }
        }

        // Fallback: scan data sections (__DATA, __DATA_CONST, __DATA_DIRTY).
        if (progress) progress("Mach-O: scanning data segments for registration patterns...");
        for (const auto& seg : segments) {
            std::string segName(seg.name);
            if (segName.find("__DATA") == std::string::npos &&
                segName.find("__DATA_CONST") == std::string::npos &&
                segName.find("__DATA_DIRTY") == std::string::npos) continue;

            if (seg.fileOffset == 0 || seg.fileSize == 0) continue;
            size_t segEnd = seg.fileOffset + seg.fileSize;
            if (segEnd > data.size()) continue;

            auto sub = scanForRegistrations(data.data() + seg.fileOffset, seg.fileSize,
                                             typeDefCount, methodDefCount, imageDefCount,
                                             is64 ? 8 : 4);
            if (sub.found()) {
                result.codeRegistration = sub.codeRegistration;
                result.metadataRegistration = sub.metadataRegistration;
                return result;
            }
        }

        return result;
    }

    // Fat Mach-O: extract first slice (architecture) and resolve.
    static RegistrationPair resolveFatMachO(const std::vector<uint8_t>& data,
                                             uint32_t typeDefCount, uint32_t methodDefCount,
                                             uint32_t imageDefCount,
                                             ProgressCallback progress) {
        RegistrationPair result;
        if (data.size() < 16) return result;

        // Fat header: magic (BE) + nfat_arch.
        uint32_t nfat = readU32LE(data, 4);
        if (nfat == 0) {
            // Big-endian: read manually.
            nfat = (static_cast<uint32_t>(data[4]) << 24) |
                   (static_cast<uint32_t>(data[5]) << 16) |
                   (static_cast<uint32_t>(data[6]) << 8) |
                   static_cast<uint32_t>(data[7]);
        }

        if (nfat > 0 && data.size() >= 16) {
            // First fat_arch entry at offset 8: cputype(4) + cpusubtype(4) + offset(4) + size(4) + align(4).
            // All big-endian in fat header.
            uint32_t sliceOffset = (static_cast<uint32_t>(data[12]) << 24) |
                                   (static_cast<uint32_t>(data[13]) << 16) |
                                   (static_cast<uint32_t>(data[14]) << 8) |
                                   static_cast<uint32_t>(data[15]);
            uint32_t sliceSize = (static_cast<uint32_t>(data[16]) << 24) |
                                 (static_cast<uint32_t>(data[17]) << 16) |
                                 (static_cast<uint32_t>(data[18]) << 8) |
                                 static_cast<uint32_t>(data[19]);

            if (sliceOffset + sliceSize <= data.size() && sliceSize > 32) {
                uint32_t sliceMagic = readU32LE(data, sliceOffset);
                bool sliceIs64 = (sliceMagic == 0xFEEDFACF);
                if (progress) progress("Fat Mach-O: extracting slice at offset " +
                                       std::to_string(sliceOffset));
                return resolveMachO(data, sliceIs64, typeDefCount, methodDefCount,
                                    imageDefCount, progress);
            }
        }

        return result;
    }

    // ══════════════════════════════════════════════════════════════════════
    // NSO (Nintendo Switch) — section scan fallback
    // ══════════════════════════════════════════════════════════════════════

    static RegistrationPair resolveNSO(const std::vector<uint8_t>& data,
                                        uint32_t typeDefCount, uint32_t methodDefCount,
                                        uint32_t imageDefCount,
                                        ProgressCallback progress) {
        // NSO format is proprietary; scan entire binary for patterns.
        if (progress) progress("NSO: scanning for registration patterns...");
        return scanForRegistrations(data.data(), data.size(),
                                     typeDefCount, methodDefCount, imageDefCount, 8);
    }

    // ══════════════════════════════════════════════════════════════════════
    // WASM — section scan fallback
    // ══════════════════════════════════════════════════════════════════════

    static RegistrationPair resolveWASM(const std::vector<uint8_t>& data,
                                         uint32_t typeDefCount, uint32_t methodDefCount,
                                         uint32_t imageDefCount,
                                         ProgressCallback progress) {
        if (progress) progress("WASM: scanning for registration patterns...");
        return scanForRegistrations(data.data(), data.size(),
                                     typeDefCount, methodDefCount, imageDefCount, 4);
    }

    // ══════════════════════════════════════════════════════════════════════
    // String pattern fallback — scan for il2cpp symbol name strings
    // ══════════════════════════════════════════════════════════════════════

    static RegistrationPair stringPatternSearch(const std::vector<uint8_t>& data,
                                                 uint32_t typeDefCount, uint32_t methodDefCount,
                                                 uint32_t imageDefCount) {
        RegistrationPair result;

        const std::string needles[] = {
            "Il2CppCodeRegistration",
            "g_CodeRegistration",
            "Il2CppMetadataRegistration",
            "g_MetadataRegistration"
        };

        for (const auto& needle : needles) {
            for (size_t i = 0; i + needle.size() <= data.size(); ++i) {
                if (std::memcmp(data.data() + i, needle.data(), needle.size()) == 0) {
                    if (needle.find("Code") != std::string::npos) {
                        result.codeRegistration = i;
                    } else {
                        result.metadataRegistration = i;
                    }
                    if (result.found()) return result;
                    break; // move to next needle
                }
            }
        }

        return result;
    }
};

} // namespace omnibyte::dumper::dumper1
