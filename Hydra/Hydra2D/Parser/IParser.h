#pragma once
// ── IParser.h ──────────────────────────────────────────────────────
// Kontrak dasar untuk semua binary format parser backend (LIEF, dst).
// Backend yang implement interface ini menangani detail per-format
// (ELF, PE, Mach-O) -- caller tidak perlu tahu.
//
// Design principles:
//   - Format-agnostic: ParsedBinary struct tidak expose detail ELF/PE-specific
//   - Minimal: hanya fungsi parse() -- extension point lain ditambah saat
//     ada kebutuhan nyata, bukan antisipasi
//   - Ikuti pola IEngineProfile.h / IDumperEngine.h: abstract interface,
//     backend implement detail

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace omnibyte::hydradis {

// ── Data types ─────────────────────────────────────────────────────

/// Tipe binary format yang didukung.
enum class BinaryFormat {
    ELF,
    PE,
    MachO,
    Unknown
};

/// Representasi satu section/segment dalam binary.
struct SectionInfo {
    std::string name;           // nama section (mis. ".text", ".rodata", ".bss")
    uint64_t virtualAddress = 0; // alamat virtual saat di-load ke memori
    uint64_t fileOffset = 0;    // offset dalam file
    uint64_t size = 0;          // ukuran dalam byte
    uint32_t flags = 0;         // attribute flags (READ, WRITE, EXECUTE, dst)
};

/// Representasi satu symbol dalam binary.
struct SymbolInfo {
    std::string name;           // nama symbol (demangled jika tersedia)
    uint64_t value = 0;         // alamat/offset symbol
    uint64_t size = 0;          // ukuran symbol dalam byte
    uint32_t type = 0;          // symbol type (FUNC, OBJECT, SECTION, dst)
    uint32_t binding = 0;       // symbol binding (LOCAL, GLOBAL, WEAK)
    int sectionIndex = -1;      // index section yang milik symbol (-1 = undefined)
};

/// Header info generik dari binary (subset yang berguna untuk analysis).
struct BinaryHeader {
    BinaryFormat format = BinaryFormat::Unknown;
    uint16_t machine = 0;       // machine type (ARM, AARCH64, AMD64, dst)
    uint64_t entryPoint = 0;    // entry point address
    uint64_t imageBase = 0;     // base address binary (ELF: biasanya 0, PE: ImageBase)
    bool is64Bit = true;        // true = 64-bit binary, false = 32-bit
    bool isEndianLittle = true; // true = little-endian, false = big-endian
};

/// Hasil parse dari satu binary file.
struct ParsedBinary {
    bool success = false;
    std::string errorMessage;   // kosong kalau success == true
    BinaryHeader header;
    std::vector<SectionInfo> sections;
    std::vector<SymbolInfo> symbols;
};

// ── Interface ──────────────────────────────────────────────────────

/// Abstract interface untuk binary format parser backend.
/// LiefParser, dst implement ini.
///
/// Usage:
///   std::unique_ptr<IParser> parser = /* factory */;
///   auto result = parser->parseFile("/path/to/libil2cpp.so");
///   for (auto& sym : result.symbols) { ... }
class IParser {
public:
    virtual ~IParser() = default;

    /// Nama backend (mis. "lief", "ghidra") -- untuk logging/diagnostic.
    virtual std::string name() const = 0;

    /// Parse binary dari file path.
    ///
    /// @param filePath path absolut ke file binary (APK, .so, .dll, .exe)
    /// @return ParsedBinary dengan header, sections, symbols atau errorMessage
    virtual ParsedBinary parseFile(const std::string& filePath) const = 0;

    /// Parse binary dari buffer in-memory.
    ///
    /// @param data     raw bytes binary
    /// @param dataSize panjang data dalam byte
    /// @return ParsedBinary dengan header, sections, symbols atau errorMessage
    virtual ParsedBinary parseBuffer(
        const uint8_t* data,
        size_t dataSize
    ) const = 0;

    /// Convenience overload: parse dari vector<uint8_t>.
    ParsedBinary parseBuffer(const std::vector<uint8_t>& data) const {
        return parseBuffer(data.data(), data.size());
    }
};

} // namespace omnibyte::hydradis
