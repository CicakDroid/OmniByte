#pragma once
#include <cstdint>
#include <cstddef>
#include <string>

namespace omnibyte::dumper {

// Kontrak profile per-versi SDK.
// Analyzer/Resolver TIDAK BOLEH hardcode offset struct di logic utamanya --
// semua offset harus lewat profile ini, supaya nambah versi baru = tambah
// satu file profile, bukan edit logic Analyzer/Resolver yang sudah ada.
class IEngineProfile {
public:
    virtual ~IEngineProfile() = default;

    // Identitas versi, mis. "27", "2022.3", "UE5.3"
    virtual std::string version() const = 0;

    // Offset generik dalam struct/header, dikunci pakai key string
    // (mis. "TypeDefinitionSizesOffset" untuk IL2CPP, "GNames" untuk Unreal).
    virtual uint64_t offsetOf(const std::string& fieldKey) const = 0;

    // Ukuran struct/header spesifik versi ini, dalam byte.
    virtual size_t structSize(const std::string& structKey) const = 0;

    // Double-check: apakah header bytes benar-benar cocok profile ini.
    // Dipakai sebagai validasi kedua setelah deteksi versi dari signature awal --
    // mencegah false-positive kalau nomor versi terdeteksi salah/di-strip.
    virtual bool validate(const uint8_t* headerBytes, size_t len) const = 0;
};

} // namespace omnibyte::dumper