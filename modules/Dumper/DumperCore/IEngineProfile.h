#pragma once
#include <cstdint>
#include <cstddef>
#include <string>
#include <optional>

namespace omnibyte::dumper {

// ── AOB (Array of Bytes) pattern untuk resolve global pointer di runtime ──
// UE global seperti GNames, GObjects, GWorld tidak punya offset statis di
// binary -- mereka di-resolve lewat pattern scanning di memori proses live.
// Struct ini menyimpan AOB pattern + cara extract address dari match result.
//
// Scanner (nantinya di SharedUtils) akan:
//   1. Scan memory section pakai bytePattern + mask
//   2. Kalau match, baca 4-byte RIP-relative offset di (matchAddr + addressOffsetFromMatch)
//   3. Hitung alamat final: (matchAddr + addressOffsetFromMatch + 4) + ripRelativeOffset
//
// Dipakai dari Engines/UnrealEngine/Resolver (runtime), bukan Analyzer (static).
struct AOBPattern {
    std::string bytePattern;          // hex pattern dgn wildcard, mis. "48 8B 05 ?? ?? ?? ??"
    std::string mask;                 // 'x' = exact, '?' = wildcard, mis. "xx?????x"
    int addressOffsetFromMatch;       // offset dari awal match ke operand RIP-relative (biasanya 3)
    int instructionLength;            // panjang instruction (biasanya 7 untuk mov rax,[rip+disp32])
};

// ── Kontrak profile per-versi SDK ──
//
// Dua method utama:
//   - offsetOf(key):  offset statis dalam struct/header (64-bit / arm64-v8a).
//                     Dipakai dari Analyzer (static analysis, baca file/binary).
//                     Contoh: "UObjectFlags" offset, "FChunkedFixedUObjectArray.ObjectsOffset".
//
//   - patternFor(key): AOB pattern untuk resolve global pointer di runtime.
//                      Dipakai dari Resolver (live process, pattern scan).
//                      Contoh: "GNamesPattern", "GObjectsPattern", "GWorldPattern".
//                      Default return nullopt -- engine yg resolve globals lewat
//                      symbol export/xDL (IL2CPP, Mono, Godot, Source) tidak perlu override.
//
// Analyzer/Resolver TIDAK BOLEH hardcode offset struct di logic utamanya --
// semua offset harus lewat profile ini, supaya nambah versi baru = tambah
// satu file profile, bukan edit logic Analyzer/Resolver yang sudah ada.
class IEngineProfile {
public:
    virtual ~IEngineProfile() = default;

    // Identitas versi, mis. "27", "2022.3", "UE5.3"
    virtual std::string version() const = 0;

    // Offset statis dalam struct/header (64-bit / arm64-v8a), dikunci pakai key string.
    // Dipakai untuk data yang punya offset tetap per versi (struct layout, field offsets).
    // Contoh: "UObjectFlags" (offset 0x8), "FChunkedObjects.ObjectsOffset" (offset 0x0).
    virtual uint64_t offsetOf(const std::string& fieldKey) const = 0;

    // Ukuran struct/header spesifik versi ini (64-bit / arm64-v8a), dalam byte.
    // Contoh: "FNameEntry" (0x10), "FUObjectItem" (0x18).
    virtual size_t structSize(const std::string& structKey) const = 0;

    // AOB pattern untuk resolve global pointer di runtime.
    // Return nullopt kalau engine ini tidak butuh AOB scan (default).
    // Contoh key: "GNamesPattern", "GObjectsPattern", "GWorldPattern".
    // Dipakai dari Resolver, bukan Analyzer.
    virtual std::optional<AOBPattern> patternFor(const std::string& patternKey) const {
        return std::nullopt;
    }

    virtual uint64_t offsetOf32(const std::string& fieldKey) const { return 0; }
    virtual size_t structSize32(const std::string& structKey) const { return 0; }

    // Double-check: apakah header bytes benar-benar cocok profile ini.
    // Dipakai sebagai validasi kedua setelah deteksi versi dari signature awal --
    // mencegah false-positive kalau nomor versi terdeteksi salah/di-strip.
    virtual bool validate(const uint8_t* headerBytes, size_t len) const = 0;
};

} // namespace omnibyte::dumper