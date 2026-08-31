#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <optional>
#include <unordered_map>

namespace omnibyte::dumper {

// === Entry types yang bisa di-dump ===

// Representasi satu type/class/struct yang ditemukan oleh engine.
struct TypeEntry {
    std::string name;                   // fully qualified name (mis. "UnityEngine.GameObject")
    uint64_t address = 0;               // alamat di memori / offset di file (0 jika belum resolved)
    size_t size = 0;                    // ukuran struct/type dalam byte
    uint32_t typeId = 0;               // ID numerik (IL2CPP: typeIndex, UE: object index)
    std::string parentType;             // nama parent/superclass (kosong jika root)
    std::vector<std::string> interfaces; // interface yang diimplement
};

// Representasi satu method/function yang ditemukan.
struct MethodEntry {
    std::string name;                   // nama method (mis. "GetComponent<T>")
    std::string declaringType;          // nama type yang declare method ini
    uint64_t address = 0;               // alamat resolved (0 jika belum resolved)
    uint32_t methodIndex = 0;          // method index dalam type
    std::string signature;              // raw signature string (opsional, engine-specific)
    bool isVirtual = false;
    bool isStatic = false;
};

// Representasi satu field/property.
struct FieldEntry {
    std::string name;                   // nama field
    std::string declaringType;          // type yang declare field ini
    uint64_t offset = 0;               // offset dalam struct (0 jika belum diketahui)
    std::string typeName;               // tipe data field (mis. "int32_t", "FString")
    size_t fieldSize = 0;              // ukuran field dalam byte
};

// Representasi string literal yang ditemukan (berguna untuk dump global-metadata).
struct StringEntry {
    std::string value;                  // isi string
    uint64_t address = 0;              // alamat di memori/file
};

// === DumpResult ===

// Hasil dump dari satu engine. Engine::analyze() mengisi bagian statis
// (typeTable, methodTable, fieldTable, strings), engine::resolveSymbols()
// mengisi address konkret (via runtime/MemoryIO).
//
// ExportCore membaca struct ini untuk menghasilkan output final
// (CSharp headers, JSON, DummyDLL, dll).
struct DumpResult {
    bool success = false;
    std::string engineName;             // nama engine yang menghasilkan dump ini
    std::string detectedVersion;        // versi yang terdeteksi (mis. "27", "5.3")

    // === Dump data ===
    std::vector<TypeEntry> typeTable;
    std::vector<MethodEntry> methodTable;
    std::vector<FieldEntry> fieldTable;
    std::vector<StringEntry> stringTable;

    // === Metadata engine-specific (opsional) ===
    // Dipakai oleh engine tertentu untuk data yang tidak cocok dengan
    // tipe-tipe generik di atas (mis. UE: GNames chunk, Godot: PCK entries).
    std::unordered_map<std::string, std::string> metadata;

    // === Error info ===
    std::string errorMessage;           // kosong jika success == true

    // Helper: tambah metadata khusus engine
    void setMeta(const std::string& key, const std::string& value) {
        metadata[key] = value;
    }

    std::optional<std::string> getMeta(const std::string& key) const {
        auto it = metadata.find(key);
        if (it != metadata.end()) return it->second;
        return std::nullopt;
    }
};

} // namespace omnibyte::dumper
