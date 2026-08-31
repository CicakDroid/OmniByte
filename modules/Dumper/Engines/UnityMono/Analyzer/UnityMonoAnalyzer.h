#pragma once
// UnityMono — static metadata analyzer for Mono images.
// Parses MonoImage → MonoClass → MonoMethod/MonoClassField from
// raw metadata bytes (file or process memory) using profile offsets.
#include "../../../DumperCore/IDumperEngine.h"
#include "../../../DumperCore/IEngineProfile.h"
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

namespace omnibyte::dumper::unitymono {

// Mono type enum values (from mono/metadata/class-internals.h)
enum class MonoTypeEnum : uint8_t {
    End         = 0x00,
    Void        = 0x01,
    Boolean     = 0x02,
    Char        = 0x03,
    I1          = 0x04,
    U1          = 0x05,
    I2          = 0x06,
    U2          = 0x07,
    I4          = 0x08,
    U4          = 0x09,
    I8          = 0x0A,
    U8          = 0x0B,
    R4          = 0x0C,
    R8          = 0x0D,
    String      = 0x0E,
    Ptr         = 0x0F,
    ByRef       = 0x10,
    ValueType   = 0x11,
    Class       = 0x12,
    Var         = 0x13,
    Array       = 0x14,
    GenericInst = 0x15,
    TypedByRef  = 0x16,
    I           = 0x18,
    U           = 0x19,
    FnPtr       = 0x1B,
    Object      = 0x1C,
    SZArray     = 0x1D,
    MVar        = 0x1E,
    CModReqd    = 0x1F,
    CModOpt     = 0x20,
    Internal    = 0x21,
    Modifier    = 0x40,
    Sentinel    = 0x41,
    Pinned      = 0x45,
    Enum        = 0x55
};

// Mono method flag bits
enum class MonoMethodFlag : uint16_t {
    Private               = 0x0001,
    FamANDAssem           = 0x0002,
    Assembly              = 0x0003,
    FamORAssem            = 0x0004,
    Static                = 0x0010,
    Final                 = 0x0020,
    Virtual               = 0x0040,
    HideBySig             = 0x0080,
    VtableLayoutMask      = 0x0100,
    NewSlot               = 0x0100,
    CheckAccessOnOverride = 0x0200,
    Abstract              = 0x0400,
    SpecialName           = 0x0800,
    PInvokeImpl           = 0x1000,
    UnmanagedExport       = 0x2000,
    RTSpecialName         = 0x4000,
    MarshalByRef          = 0x8000
};

class UnityMonoAnalyzer {
public:
    static DumpResult analyze(const AnalysisTarget& target,
                              const std::shared_ptr<IEngineProfile>& profile) {
        DumpResult result;
        result.engineName = "Unity Mono";
        result.detectedVersion = profile ? profile->version() : "unknown";

        if (!profile) {
            result.errorMessage = "No profile provided";
            return result;
        }

        std::vector<uint8_t> buffer;
        if (target.isFile()) {
            buffer = readFile(target.filePath);
            if (buffer.empty()) {
                result.errorMessage = "Failed to read file: " + target.filePath;
                return result;
            }
        } else {
            result.errorMessage = "Process target not yet implemented (requires /proc/pid/mem reader)";
            return result;
        }

        return parseMetadata(buffer, profile);
    }

private:
    static std::vector<uint8_t> readFile(const std::string& path) {
        FILE* f = fopen(path.c_str(), "rb");
        if (!f) return {};
        fseek(f, 0, SEEK_END);
        long sz = ftell(f);
        if (sz <= 0) { fclose(f); return {}; }
        fseek(f, 0, SEEK_SET);
        std::vector<uint8_t> buf(static_cast<size_t>(sz));
        size_t read = fread(buf.data(), 1, buf.size(), f);
        fclose(f);
        buf.resize(read);
        return buf;
    }

    // Read a pointer (8 bytes on 64-bit) from buffer at offset
    static uint64_t readPtr(const std::vector<uint8_t>& buf, size_t offset) {
        if (offset + 8 > buf.size()) return 0;
        uint64_t val = 0;
        memcpy(&val, buf.data() + offset, 8);
        return val;
    }

    static uint32_t readU32(const std::vector<uint8_t>& buf, size_t offset) {
        if (offset + 4 > buf.size()) return 0;
        uint32_t val = 0;
        memcpy(&val, buf.data() + offset, 4);
        return val;
    }

    static uint16_t readU16(const std::vector<uint8_t>& buf, size_t offset) {
        if (offset + 2 > buf.size()) return 0;
        uint16_t val = 0;
        memcpy(&val, buf.data() + offset, 2);
        return val;
    }

    static uint8_t readU8(const std::vector<uint8_t>& buf, size_t offset) {
        if (offset >= buf.size()) return 0;
        return buf[offset];
    }

    // Read a C string from buffer at the given offset (dereferences a pointer
    // stored in the metadata struct, then looks it up in the string heap).
    // For static metadata, the "pointer" is actually an offset into raw_data.
    static std::string readString(const std::vector<uint8_t>& buf,
                                  size_t ptrOffset,
                                  uint64_t rawDataBase) {
        uint64_t strPtr = readPtr(buf, ptrOffset);
        if (strPtr == 0) return "";
        // In static metadata files, pointers are relative to raw_data start.
        // Adjust: strPtr is an absolute VA in the struct, but in the file
        // it's stored as offset from raw_data base.
        size_t fileOffset = static_cast<size_t>(strPtr - rawDataBase);
        if (fileOffset >= buf.size()) return "";
        const char* start = reinterpret_cast<const char*>(buf.data() + fileOffset);
        size_t maxLen = buf.size() - fileOffset;
        return std::string(start, strnlen(start, maxLen));
    }

    // Read string from the string heap using a heap offset
    static std::string readHeapString(const std::vector<uint8_t>& buf,
                                      uint32_t heapOffset,
                                      size_t heapBase,
                                      size_t heapSize) {
        size_t absOffset = heapBase + heapOffset;
        if (absOffset >= buf.size()) return "";
        const char* start = reinterpret_cast<const char*>(buf.data() + absOffset);
        size_t maxLen = buf.size() - absOffset;
        return std::string(start, strnlen(start, maxLen));
    }

    // Parse MonoType from buffer at given offset.
    // MonoType layout: union data(8) + attrs:16(2) + type:8(1) + mods/byref/pinned(1) = 12 bytes
    struct ParsedMonoType {
        MonoTypeEnum typeEnum = MonoTypeEnum::End;
        bool byref = false;
        bool pinned = false;
        uint16_t attrs = 0;
    };

    static ParsedMonoType parseMonoType(const std::vector<uint8_t>& buf, size_t offset) {
        ParsedMonoType pt;
        if (offset + 12 > buf.size()) return pt;
        // attrs is at offset + 8, type enum at offset + 10
        pt.attrs = readU16(buf, offset + 8);
        pt.typeEnum = static_cast<MonoTypeEnum>(readU8(buf, offset + 10));
        uint8_t modsByte = readU8(buf, offset + 11);
        pt.byref = (modsByte >> 6) & 1;
        pt.pinned = (modsByte >> 7) & 1;
        return pt;
    }

    static std::string typeNameFromEnum(MonoTypeEnum t) {
        switch (t) {
            case MonoTypeEnum::Void:        return "void";
            case MonoTypeEnum::Boolean:     return "bool";
            case MonoTypeEnum::Char:        return "char";
            case MonoTypeEnum::I1:          return "int8_t";
            case MonoTypeEnum::U1:          return "uint8_t";
            case MonoTypeEnum::I2:          return "int16_t";
            case MonoTypeEnum::U2:          return "uint16_t";
            case MonoTypeEnum::I4:          return "int32_t";
            case MonoTypeEnum::U4:          return "uint32_t";
            case MonoTypeEnum::I8:          return "int64_t";
            case MonoTypeEnum::U8:          return "uint64_t";
            case MonoTypeEnum::R4:          return "float";
            case MonoTypeEnum::R8:          return "double";
            case MonoTypeEnum::String:      return "string";
            case MonoTypeEnum::Object:      return "object";
            case MonoTypeEnum::Ptr:         return "ptr";
            case MonoTypeEnum::ByRef:       return "ref";
            case MonoTypeEnum::ValueType:   return "valuetype";
            case MonoTypeEnum::Class:       return "class";
            case MonoTypeEnum::Array:       return "array";
            case MonoTypeEnum::SZArray:     return "szarray";
            case MonoTypeEnum::GenericInst: return "generic_inst";
            case MonoTypeEnum::Var:         return "!0";
            case MonoTypeEnum::MVar:        return "!!0";
            default:                        return "unknown";
        }
    }

    // ── Mono metadata table indices (ECMA-335 / Mono) ────────────
    enum TableIndex {
        TableModule           = 0x00,
        TableTypeRef          = 0x01,
        TableTypeDef          = 0x02,
        TableFieldDef         = 0x04,
        TableMethodDef        = 0x06,
        TableParam            = 0x08,
        TableInterfaceImpl    = 0x09,
        TableMemberRef        = 0x0A,
        TableConstant         = 0x0B,
        TableCustomAttribute  = 0x0C,
        TableFieldMarshal     = 0x0D,
        TableDeclSecurity     = 0x0E,
        TableClassLayout      = 0x0F,
        TableFieldLayout      = 0x10,
        TableStandAloneSig    = 0x11,
        TableEventMap         = 0x12,
        TableEvent            = 0x13,
        TablePropertyMap      = 0x14,
        TableProperty         = 0x15,
        TableMethodSemantics  = 0x16,
        TableMethodImpl       = 0x17,
        TableModuleRef        = 0x1A,
        TableTypeSpec         = 0x1B,
        TableImplMap          = 0x1C,
        TableFieldRVA         = 0x1D,
        TableAssembly        = 0x20,
        TableAssemblyProcessor = 0x21,
        TableAssemblyRef     = 0x23,
        TableMethodPtr        = 0x25,
        TableNestedType       = 0x29,
        TableGenericParam     = 0x2A,
        TableMethodSpec       = 0x2B,
        TableGenericParamConstraint = 0x2C,
        NUM_TABLES           = 0x40
    };

    // MonoTableInfo layout (metadata-internals.h):
    //   guint32 rows;      // offset 0
    //   guint32 row_size;  // offset 4
    //   const char* data;  // offset 8 (pointer, 8 bytes on 64-bit)
    // Total: 16 bytes
    struct MonoTableInfo {
        uint32_t rows = 0;
        uint32_t row_size = 0;
        uint64_t data = 0; // file offset to table data
    };

    static MonoTableInfo readTableInfo(const std::vector<uint8_t>& buf,
                                       size_t tablesBaseOffset,
                                       int tableIndex,
                                       uint64_t rawDataBase) {
        MonoTableInfo info;
        size_t entryOffset = tablesBaseOffset + (tableIndex * 16); // MonoTableInfo_sizeof = 0x10
        info.rows = readU32(buf, entryOffset);
        info.row_size = readU32(buf, entryOffset + 4);
        info.data = readPtr(buf, entryOffset + 8);
        // Convert VA to file offset
        if (info.data != 0) {
            info.data = info.data - rawDataBase;
        }
        return info;
    }

    // Parse TypeDef row (table 0x02).
    // Row layout: flags(4) + name_idx(2/4) + namespace_idx(2/4) + extends_idx(2/4)
    //             + field_list(2/4) + method_list(2/4)
    // String index size depends on # strings in heap: 2 if < 65536, else 4.
    // Table index size depends on # rows in target table: 2 if < 65536, else 4.
    struct TypeDefRow {
        uint32_t flags = 0;
        uint32_t nameIdx = 0;
        uint32_t namespaceIdx = 0;
        uint32_t extendsIdx = 0;
        uint32_t fieldList = 0;
        uint32_t methodList = 0;
    };

    static TypeDefRow parseTypeDefRow(const std::vector<uint8_t>& buf,
                                      size_t rowOffset,
                                      bool stringIsLong,
                                      bool tableIsLong) {
        TypeDefRow row;
        size_t off = rowOffset;
        row.flags = readU32(buf, off); off += 4;
        row.nameIdx = stringIsLong ? readU32(buf, off) : readU16(buf, off);
        off += stringIsLong ? 4 : 2;
        row.namespaceIdx = stringIsLong ? readU32(buf, off) : readU16(buf, off);
        off += stringIsLong ? 4 : 2;
        row.extendsIdx = tableIsLong ? readU32(buf, off) : readU16(buf, off);
        off += tableIsLong ? 4 : 2;
        row.fieldList = tableIsLong ? readU32(buf, off) : readU16(buf, off);
        off += tableIsLong ? 4 : 2;
        row.methodList = tableIsLong ? readU32(buf, off) : readU16(buf, off);
        return row;
    }

    // Parse FieldDef row (table 0x04).
    // Row layout: flags(2) + name_idx(2/4) + signature_idx(2/4)
    struct FieldDefRow {
        uint16_t flags = 0;
        uint32_t nameIdx = 0;
        uint32_t signatureIdx = 0;
    };

    static FieldDefRow parseFieldDefRow(const std::vector<uint8_t>& buf,
                                        size_t rowOffset,
                                        bool stringIsLong) {
        FieldDefRow row;
        size_t off = rowOffset;
        row.flags = readU16(buf, off); off += 2;
        row.nameIdx = stringIsLong ? readU32(buf, off) : readU16(buf, off);
        off += stringIsLong ? 4 : 2;
        row.signatureIdx = stringIsLong ? readU32(buf, off) : readU16(buf, off);
        return row;
    }

    // Parse MethodDef row (table 0x06).
    // Row layout: RVA(4) + impl_flags(2) + flags(2) + name_idx(2/4) + signature_idx(2/4) + param_list(2/4)
    struct MethodDefRow {
        uint32_t rva = 0;
        uint16_t implFlags = 0;
        uint16_t flags = 0;
        uint32_t nameIdx = 0;
        uint32_t signatureIdx = 0;
        uint32_t paramList = 0;
    };

    static MethodDefRow parseMethodDefRow(const std::vector<uint8_t>& buf,
                                          size_t rowOffset,
                                          bool stringIsLong,
                                          bool tableIsLong) {
        MethodDefRow row;
        size_t off = rowOffset;
        row.rva = readU32(buf, off); off += 4;
        row.implFlags = readU16(buf, off); off += 2;
        row.flags = readU16(buf, off); off += 2;
        row.nameIdx = stringIsLong ? readU32(buf, off) : readU16(buf, off);
        off += stringIsLong ? 4 : 2;
        row.signatureIdx = stringIsLong ? readU32(buf, off) : readU16(buf, off);
        off += stringIsLong ? 4 : 2;
        row.paramList = tableIsLong ? readU32(buf, off) : readU16(buf, off);
        return row;
    }

    // Main metadata parse entry point
    static DumpResult parseMetadata(const std::vector<uint8_t>& buf,
                                    const std::shared_ptr<IEngineProfile>& profile) {
        DumpResult result;
        result.engineName = "Unity Mono";
        result.detectedVersion = profile->version();

        // Minimum size check
        if (buf.size() < 0x100) {
            result.errorMessage = "File too small for Mono metadata";
            return result;
        }

        // Read key MonoImage offsets from profile
        // In static files, "pointers" in MonoImage structs are actually
        // offsets relative to raw_data start. For our purposes, raw_data
        // is typically at a known offset in the file.
        //
        // For a real metadata image (libmono.so + metadata blob),
        // the metadata sits at a fixed location. We treat the entire
        // buffer as the metadata image starting at offset 0.

        // Read heap stream headers
        size_t heapStringsOffset = static_cast<size_t>(profile->offsetOf("MonoImage_heap_strings"));
        size_t heapBlobOffset = static_cast<size_t>(profile->offsetOf("MonoImage_heap_blob"));

        // MonoStreamHeader: size(4) + offset(4) = 8 bytes
        uint32_t stringHeapSize = readU32(buf, heapStringsOffset);
        uint32_t stringHeapOffset = readU32(buf, heapStringsOffset + 4);
        uint32_t blobHeapSize = readU32(buf, heapBlobOffset);
        uint32_t blobHeapOffset = readU32(buf, heapBlobOffset + 4);

        // Read tables base
        size_t tablesBaseOffset = static_cast<size_t>(profile->offsetOf("MonoImage_tables_base"));
        // tables_base is a pointer in the struct; in the file it's at tables offset
        size_t tablesOffset = static_cast<size_t>(profile->offsetOf("MonoImage_tables"));

        // Read TypeDef table info (index 0x02)
        MonoTableInfo typeDefTable = readTableInfo(buf, tablesOffset, TableTypeDef, 0);
        MonoTableInfo fieldDefTable = readTableInfo(buf, tablesOffset, TableFieldDef, 0);
        MonoTableInfo methodDefTable = readTableInfo(buf, tablesOffset, TableMethodDef, 0);

        if (typeDefTable.rows == 0 || typeDefTable.row_size == 0) {
            result.errorMessage = "TypeDef table is empty or invalid";
            return result;
        }

        // Determine string index size from # strings in string heap
        bool stringIsLong = (stringHeapSize > 0xFFFF);
        // Determine table index size from # rows in relevant tables
        bool tableIsLong = (fieldDefTable.rows > 0xFFFF || methodDefTable.rows > 0xFFFF);

        // Parse each TypeDef row
        for (uint32_t i = 0; i < typeDefTable.rows; ++i) {
            size_t rowOffset = typeDefTable.data + (i * typeDefTable.row_size);
            if (rowOffset + typeDefTable.row_size > buf.size()) break;

            TypeDefRow tdr = parseTypeDefRow(buf, rowOffset, stringIsLong, tableIsLong);

            // Read type name from string heap
            std::string typeName = readHeapString(buf, tdr.nameIdx,
                                                  stringHeapOffset, stringHeapSize);
            std::string typeNs = readHeapString(buf, tdr.namespaceIdx,
                                                stringHeapOffset, stringHeapSize);

            if (typeName.empty()) continue;

            // Build fully qualified name
            std::string fullName = typeNs.empty() ? typeName : (typeNs + "." + typeName);

            TypeEntry te;
            te.name = fullName;
            te.typeId = i;
            te.address = 0; // not yet resolved
            te.size = 0;

            // Parse fields for this type
            uint32_t fieldStart = tdr.fieldList;
            uint32_t fieldEnd = (i + 1 < typeDefTable.rows)
                ? parseTypeDefRow(buf,
                                  typeDefTable.data + ((i + 1) * typeDefTable.row_size),
                                  stringIsLong, tableIsLong).fieldList
                : (fieldDefTable.rows + 1);

            for (uint32_t fi = fieldStart; fi < fieldEnd && fi > 0; ++fi) {
                uint32_t fieldIdx = fi - 1;
                if (fieldIdx >= fieldDefTable.rows) break;
                size_t fRowOff = fieldDefTable.data + (fieldIdx * fieldDefTable.row_size);
                if (fRowOff + fieldDefTable.row_size > buf.size()) break;

                FieldDefRow fdr = parseFieldDefRow(buf, fRowOff, stringIsLong);
                std::string fieldName = readHeapString(buf, fdr.nameIdx,
                                                       stringHeapOffset, stringHeapSize);

                FieldEntry fe;
                fe.name = fieldName;
                fe.declaringType = fullName;
                fe.typeName = ""; // signature parsing is complex; leave for resolver
                fe.offset = 0;
                fe.fieldSize = 0;
                result.fieldTable.push_back(fe);
            }

            // Parse methods for this type
            uint32_t methodStart = tdr.methodList;
            uint32_t methodEnd = (i + 1 < typeDefTable.rows)
                ? parseTypeDefRow(buf,
                                  typeDefTable.data + ((i + 1) * typeDefTable.row_size),
                                  stringIsLong, tableIsLong).methodList
                : (methodDefTable.rows + 1);

            for (uint32_t mi = methodStart; mi < methodEnd && mi > 0; ++mi) {
                uint32_t methodIdx = mi - 1;
                if (methodIdx >= methodDefTable.rows) break;
                size_t mRowOff = methodDefTable.data + (methodIdx * methodDefTable.row_size);
                if (mRowOff + methodDefTable.row_size > buf.size()) break;

                MethodDefRow mdr = parseMethodDefRow(buf, mRowOff, stringIsLong, tableIsLong);
                std::string methodName = readHeapString(buf, mdr.nameIdx,
                                                        stringHeapOffset, stringHeapSize);

                MethodEntry me;
                me.name = methodName;
                me.declaringType = fullName;
                me.methodIndex = mi - methodStart;
                me.address = 0;
                me.isStatic = (mdr.flags & static_cast<uint16_t>(MonoMethodFlag::Static)) != 0;
                me.isVirtual = (mdr.flags & static_cast<uint16_t>(MonoMethodFlag::Virtual)) != 0;
                result.methodTable.push_back(me);
            }

            result.typeTable.push_back(te);
        }

        result.success = !result.typeTable.empty();
        if (!result.success) {
            result.errorMessage = "No types found in metadata tables";
        }

        return result;
    }
};

} // namespace omnibyte::dumper::unitymono
