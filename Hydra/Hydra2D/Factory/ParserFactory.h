#pragma once
// ── ParserFactory.h ───────────────────────────────────────────────
// Factory untuk membuat IParser instance.
// Saat ini hanya LIEF, tapi factory menambah layer abstraksi
// sehingga backend lain bisa ditambah tanpa ubah caller.

#include "Parser/IParser.h"
#include <memory>
#include <string>

namespace omnibyte::hydradis {

/// Backend pilihan untuk parsing.
enum class ParserBackend {
    Lief,   // default — ELF-focused, ringan
};

/// Factory untuk IParser.
///
/// Usage:
///   auto parser = ParserFactory::create();
///   auto result = parser->parseFile("/path/to/libil2cpp.so");
class ParserFactory {
public:
    /// Buat IParser instance.
    ///
    /// @param backend  backend pilihan (default: Lief)
    /// @return unique_ptr ke IParser
    static std::unique_ptr<IParser> create(
        ParserBackend backend = ParserBackend::Lief
    );
};

} // namespace omnibyte::hydradis
