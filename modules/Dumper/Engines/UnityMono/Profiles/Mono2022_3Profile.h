#pragma once
// ── Profiles/Mono2022_3Profile.h ──────────────────────────────────
// Unity 2022.3 LTS ships Mono 2022.3.x (based on Mono runtime ~6.12.x).
// Struct layouts derived from mono/mono@mono-2022-07 tag:
//   metadata-internals.h, class-private-definition.h, class-internals.h
// Targets: 64-bit (arm64-v8a), 32-bit (armeabi-v7a).
//
// NOTE: Unity maintains a fork of Mono. Struct layouts here match
// the upstream mono/mono struct definitions. If Unity patches fields,
// offsets may differ — report to maintainer.
#include "../../../DumperCore/IEngineProfile.h"
#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>

namespace omnibyte::dumper::unitymono {

class Mono2022_3Profile : public IEngineProfile {
public:
    std::string version() const override { return "2022.3"; }

    uint64_t offsetOf(const std::string& key) const override {
        static const std::unordered_map<std::string, uint64_t> kOffsets = {
            // ── MonoImage (metadata-internals.h) ────────────────────
            // MonoImage is the central metadata container.
            // Source: mono/metadata/metadata-internals.h
            {"MonoImageOffset",                 0x0},    // not a field offset; used as marker
            {"MonoImage_sizeof",                0x390},  // approximate; varies by build flags

            // Raw metadata pointers
            {"MonoImage_raw_data",              0x10},   // char* raw_data
            {"MonoImage_raw_data_len",          0x18},   // guint32 raw_data_len

            // Identity strings
            {"MonoImage_name",                  0x30},   // char* name
            {"MonoImage_filename",              0x38},   // char* filename
            {"MonoImage_assembly_name",         0x40},   // const char* assembly_name
            {"MonoImage_module_name",           0x48},   // const char* module_name
            {"MonoImage_version",               0x58},   // char* version
            {"MonoImage_guid",                  0x68},   // char* guid

            // Metadata version
            {"MonoImage_md_version_major",      0x70},   // gint16
            {"MonoImage_md_version_minor",      0x72},   // gint16

            // Heap stream headers (each: guint32 size + guint32 offset = 8 bytes)
            {"MonoImage_heap_strings",          0x88},   // MonoStreamHeader
            {"MonoImage_heap_us",               0x90},   // MonoStreamHeader
            {"MonoImage_heap_blob",             0x98},   // MonoStreamHeader
            {"MonoImage_heap_guid",             0xA0},   // MonoStreamHeader
            {"MonoImage_heap_tables",           0xA8},   // MonoStreamHeader

            // Tables
            {"MonoImage_tables_base",           0xB0},   // const char* tables_base
            {"MonoImage_tables",                0xB8},   // MonoTableInfo tables[MONO_TABLE_NUM]
            // MonoTableInfo is 3 x guint32 (rows, row_size, data pointer) = 16 bytes each.
            // MONO_TABLE_NUM = 64 (as of Mono 6.x).
            {"MonoTableInfo_sizeof",            0x10},   // 16 bytes per table entry

            // Assembly reference
            {"MonoImage_assembly",              0x318},  // MonoAssembly* assembly (approx)

            // ── MonoClass (class-private-definition.h) ──────────────
            // Source: mono/metadata/class-private-definition.h
            // 64-bit layout — pointers are 8 bytes.
            {"MonoClass_sizeof",                0x118},  // 280 bytes on 64-bit

            // Pointers / identity
            {"MonoClass_element_class",         0x00},   // MonoClass*
            {"MonoClass_cast_class",            0x08},   // MonoClass*
            {"MonoClass_supertypes",            0x10},   // MonoClass**
            {"MonoClass_idepth",                0x18},   // guint16
            {"MonoClass_rank",                  0x1A},   // guint8
            {"MonoClass_class_kind",            0x1B},   // guint8 (MonoTypeKind)
            {"MonoClass_instance_size",         0x1C},   // int

            // Bitfields (packed into ~4 bytes at 0x20)
            // inited:1, size_inited:1, valuetype:1, enumtype:1, blittable:1,
            // unicode:1, wastypebuilder:1, is_array_special_interface:1, is_byreflike:1
            {"MonoClass_bitfield1",             0x20},   // 9 bits packed

            {"MonoClass_min_align",             0x24},   // guint8

            // packing_size:4, ghcimpl:1, has_finalize:1, marshalbyref:1, contextbound:1
            {"MonoClass_bitfield2",             0x25},   // 8 bits packed

            // delegate:1, gc_descr_inited:1, has_cctor:1, has_references:1,
            // has_static_refs:1, no_special_static_fields:1, is_com_object:1, nested_classes_inited:1
            {"MonoClass_bitfield3",             0x26},   // 8 bits packed

            // interfaces_inited:1, simd_type:1, has_finalize_inited:1, fields_inited:1,
            // has_failure:1, has_weak_fields:1, has_dim_conflicts:1
            {"MonoClass_bitfield4",             0x27},   // 7 bits packed

            // Parent / image / identity
            {"MonoClass_parent",                0x28},   // MonoClass*
            {"MonoClass_nested_in",             0x30},   // MonoClass*
            {"MonoClass_image",                 0x38},   // MonoImage*
            {"MonoClass_name",                  0x40},   // const char*
            {"MonoClass_name_space",            0x48},   // const char*

            // Token / vtable
            {"MonoClass_type_token",            0x50},   // guint32
            {"MonoClass_vtable_size",           0x54},   // int

            // Interfaces
            {"MonoClass_interface_count",       0x58},   // guint16
            {"MonoClass_interface_id",          0x5C},   // guint32 (after 2-byte pad)
            {"MonoClass_max_interface_id",      0x60},   // guint32
            {"MonoClass_interface_offsets_count", 0x64}, // guint16
            {"MonoClass_interfaces_packed",     0x68},   // MonoClass**
            {"MonoClass_interface_offsets_packed", 0x70},// guint16*
            {"MonoClass_interface_bitmap",      0x78},   // guint8*
            {"MonoClass_interfaces",            0x80},   // MonoClass**

            // Sizes union
            {"MonoClass_sizes",                 0x88},   // union { class_size, element_size, generic_param_token }

            // TypeDef table info
            {"MonoClass_flags",                 0x8C},   // guint32
            {"MonoClass_field_first",           0x90},   // guint32 (first field index)
            {"MonoClass_field_count",           0x94},   // guint32
            {"MonoClass_method_first",          0x98},   // guint32 (first method index)
            {"MonoClass_method_count",          0x9C},   // guint32

            {"MonoClass_ref_info_handle",       0xA0},   // guint32
            {"MonoClass_marshal_info",          0xA8},   // MonoMarshalType* (after pad)

            // Field / method arrays
            {"MonoClass_fields",                0xB0},   // MonoClassField*
            {"MonoClass_methods",               0xB8},   // MonoMethod**

            // Type signatures
            {"MonoClass_this_arg",              0xC0},   // MonoType
            {"MonoClass__byval_arg",            0xD0},   // MonoType

            // Generic / runtime
            {"MonoClass_generic_class",         0xE0},   // MonoGenericClass*
            {"MonoClass_generic_container",     0xE8},   // MonoGenericContainer*
            {"MonoClass_gc_descr",              0xF0},   // MonoGCDescriptor
            {"MonoClass_runtime_info",          0xF8},   // MonoClassRuntimeInfo*
            {"MonoClass_next_class_cache",      0x100},  // MonoClass*
            {"MonoClass_vtable",                0x108},  // MonoMethod**
            {"MonoClass_ext",                   0x110},  // MonoClassExt*

            // ── MonoClassField (class-internals.h) ──────────────────
            // Source: mono/metadata/class-internals.h
            {"MonoClassField_sizeof",           0x20},   // 32 bytes (padded)

            {"MonoClassField_type",             0x00},   // MonoType*
            {"MonoClassField_name",             0x08},   // const char*
            {"MonoClassField_parent",           0x10},   // MonoClass*
            {"MonoClassField_offset",           0x18},   // int (field offset from object start)

            // ── MonoMethod (class-internals.h) ──────────────────────
            // Source: mono/metadata/class-internals.h
            {"MonoMethod_sizeof",               0x28},   // 40 bytes on 64-bit

            {"MonoMethod_flags",                0x00},   // guint16 (method flags)
            {"MonoMethod_iflags",               0x02},   // guint16 (impl flags)
            {"MonoMethod_token",                0x04},   // guint32
            {"MonoMethod_klass",                0x08},   // MonoClass*
            {"MonoMethod_signature",            0x10},   // MonoMethodSignature*
            {"MonoMethod_name",                 0x18},   // const char*

            // Bitfields at 0x20: inline_info:1, inline_failure:1, wrapper_type:5,
            // string_ctor:1, save_lmf:1, dynamic:1, is_generic:1, is_inflated:1,
            // skip_visibility:1, verification_success:1, is_mb_open:1
            {"MonoMethod_bitfields",            0x20},   // 17 bits packed
            {"MonoMethod_slot",                 0x24},   // signed int :17 (padded to 4 bytes)

            // ── MonoType (class-internals.h) ────────────────────────
            // Source: mono/metadata/class-internals.h
            {"MonoType_sizeof",                 0x10},   // 16 bytes on 64-bit (padded)

            {"MonoType_data",                   0x00},   // union { klass, type, array, method, generic_param, generic_class }
            {"MonoType_attrs",                  0x08},   // guint16 (bitfield)
            {"MonoType_type",                   0x0A},   // guint8 (MonoTypeEnum, bitfield)
            {"MonoType_num_mods",               0x0B},   // guint6 (bitfield)
            {"MonoType_byref",                  0x0B},   // guint1 (bitfield, packed with num_mods)
            {"MonoType_pinned",                 0x0B},   // guint1 (bitfield, packed with num_mods)
        };
        auto it = kOffsets.find(key);
        return it != kOffsets.end() ? it->second : 0;
    }

    size_t structSize(const std::string& key) const override {
        static const std::unordered_map<std::string, size_t> kSizes = {
            {"MonoImage",    0x390},   // approximate; varies by build flags
            {"MonoClass",    0x118},   // 280 bytes
            {"MonoClassField", 0x20},  // 32 bytes
            {"MonoMethod",   0x28},    // 40 bytes
            {"MonoType",     0x10},    // 16 bytes
            {"MonoStreamHeader", 0x08},// 8 bytes (guint32 size + guint32 offset)
            {"MonoTableInfo", 0x10},   // 16 bytes (guint32 rows + guint32 row_size + void* data)
        };
        auto it = kSizes.find(key);
        return it != kSizes.end() ? it->second : 0;
    }

    uint64_t offsetOf32(const std::string& key) const override {
        static const std::unordered_map<std::string, uint64_t> kOffsets = {
            // ── MonoImage (32-bit, armeabi-v7a) ────────────────────
            // Pointers shrink 8→4; MonoStreamHeader stays 8 (2× guint32).
            // MonoTableInfo shrinks 16→12 (3× guint32).
            // NOTE: Unity fork may add fields not in upstream — offsets marked
            // approx need runtime verification.
            {"MonoImage_sizeof",                0x228},  // approx from upstream32 layout

            {"MonoImage_raw_data",              0x08},   // char*
            {"MonoImage_raw_data_len",          0x0C},   // guint32

            {"MonoImage_name",                  0x18},   // char*
            {"MonoImage_filename",              0x1C},   // char*
            {"MonoImage_assembly_name",         0x20},   // const char*
            {"MonoImage_module_name",           0x24},   // const char*
            {"MonoImage_version",               0x2C},   // char*
            {"MonoImage_guid",                  0x34},   // char*

            {"MonoImage_md_version_major",      0x38},   // gint16
            {"MonoImage_md_version_minor",      0x3A},   // gint16

            {"MonoImage_heap_strings",          0x44},   // MonoStreamHeader (8 bytes)
            {"MonoImage_heap_us",               0x4C},   // MonoStreamHeader
            {"MonoImage_heap_blob",             0x54},   // MonoStreamHeader
            {"MonoImage_heap_guid",             0x5C},   // MonoStreamHeader
            {"MonoImage_heap_tables",           0x64},   // MonoStreamHeader

            {"MonoImage_tables_base",           0x6C},   // const char*
            {"MonoImage_tables",                0x70},   // MonoTableInfo[64]
            {"MonoTableInfo_sizeof",            0x0C},   // 12 bytes on 32-bit

            {"MonoImage_assembly",              0x1F8},  // approx; Unity fork offset

            // ── MonoClass (32-bit) ─────────────────────────────────
            // All pointers 4 bytes; MonoType 8 bytes (vs 16 on 64-bit).
            {"MonoClass_sizeof",                0xA0},

            {"MonoClass_element_class",         0x00},   // MonoClass*
            {"MonoClass_cast_class",            0x04},   // MonoClass*
            {"MonoClass_supertypes",            0x08},   // MonoClass**
            {"MonoClass_idepth",                0x0C},   // guint16
            {"MonoClass_rank",                  0x0E},   // guint8
            {"MonoClass_class_kind",            0x0F},   // guint8
            {"MonoClass_instance_size",         0x10},   // int

            {"MonoClass_bitfield1",             0x14},   // packed bits
            {"MonoClass_min_align",             0x18},   // guint8
            {"MonoClass_bitfield2",             0x19},
            {"MonoClass_bitfield3",             0x1A},
            {"MonoClass_bitfield4",             0x1B},

            {"MonoClass_parent",                0x1C},   // MonoClass*
            {"MonoClass_nested_in",             0x20},   // MonoClass*
            {"MonoClass_image",                 0x24},   // MonoImage*
            {"MonoClass_name",                  0x28},   // const char*
            {"MonoClass_name_space",            0x2C},   // const char*

            {"MonoClass_type_token",            0x30},   // guint32
            {"MonoClass_vtable_size",           0x34},   // int

            {"MonoClass_interface_count",       0x38},   // guint16
            {"MonoClass_interface_id",          0x3C},   // guint32
            {"MonoClass_max_interface_id",      0x40},   // guint32
            {"MonoClass_interface_offsets_count",0x44},  // guint16
            {"MonoClass_interfaces_packed",     0x48},   // MonoClass**
            {"MonoClass_interface_offsets_packed",0x4C}, // guint16*
            {"MonoClass_interface_bitmap",      0x50},   // guint8*
            {"MonoClass_interfaces",            0x54},   // MonoClass**

            {"MonoClass_sizes",                 0x58},   // union (4 bytes)
            {"MonoClass_flags",                 0x5C},   // guint32
            {"MonoClass_field_first",           0x60},   // guint32
            {"MonoClass_field_count",           0x64},   // guint32
            {"MonoClass_method_first",          0x68},   // guint32
            {"MonoClass_method_count",          0x6C},   // guint32

            {"MonoClass_ref_info_handle",       0x70},   // guint32
            {"MonoClass_marshal_info",          0x74},   // MonoMarshalType*

            {"MonoClass_fields",                0x78},   // MonoClassField*
            {"MonoClass_methods",               0x7C},   // MonoMethod**

            {"MonoClass_this_arg",              0x80},   // MonoType (8 bytes)
            {"MonoClass__byval_arg",            0x88},   // MonoType (8 bytes)

            {"MonoClass_generic_class",         0x90},   // MonoGenericClass*
            {"MonoClass_generic_container",     0x94},   // MonoGenericContainer*
            {"MonoClass_gc_descr",              0x98},   // MonoGCDescriptor
            {"MonoClass_runtime_info",          0x9C},   // MonoClassRuntimeInfo*
            {"MonoClass_next_class_cache",      0xA0},   // MonoClass* (end of struct)
            {"MonoClass_vtable",                0xA4},   // MonoMethod**
            {"MonoClass_ext",                   0xA8},   // MonoClassExt*

            // ── MonoClassField (32-bit) ────────────────────────────
            // Pointers 4 bytes; sizeof shrinks 0x20→0x10.
            {"MonoClassField_sizeof",           0x10},

            {"MonoClassField_type",             0x00},   // MonoType* (inline, 8 bytes)
            {"MonoClassField_name",             0x04},   // const char*
            {"MonoClassField_parent",           0x08},   // MonoClass*
            {"MonoClassField_offset",           0x0C},   // int

            // ── MonoMethod (32-bit) ────────────────────────────────
            // Pointers 4 bytes; sizeof shrinks 0x28→0x1C.
            {"MonoMethod_sizeof",               0x1C},

            {"MonoMethod_flags",                0x00},   // guint16
            {"MonoMethod_iflags",               0x02},   // guint16
            {"MonoMethod_token",                0x04},   // guint32
            {"MonoMethod_klass",                0x08},   // MonoClass*
            {"MonoMethod_signature",            0x0C},   // MonoMethodSignature*
            {"MonoMethod_name",                 0x10},   // const char*
            {"MonoMethod_bitfields",            0x14},   // packed bits
            {"MonoMethod_slot",                 0x18},   // signed int

            // ── MonoType (32-bit) ──────────────────────────────────
            // sizeof shrinks 0x10→0x08.
            {"MonoType_sizeof",                 0x08},

            {"MonoType_data",                   0x00},   // union (4 bytes)
            {"MonoType_attrs",                  0x04},   // guint16
            {"MonoType_type",                   0x06},   // guint8 (bitfield)
            {"MonoType_num_mods",               0x07},   // guint6 (bitfield)
            {"MonoType_byref",                  0x07},   // guint1 (packed)
            {"MonoType_pinned",                 0x07},   // guint1 (packed)
        };
        auto it = kOffsets.find(key);
        return it != kOffsets.end() ? it->second : 0;
    }

    size_t structSize32(const std::string& key) const override {
        static const std::unordered_map<std::string, size_t> kSizes = {
            {"MonoImage",        0x228},
            {"MonoClass",        0xA0},
            {"MonoClassField",   0x10},
            {"MonoMethod",       0x1C},
            {"MonoType",         0x08},
            {"MonoStreamHeader", 0x08},
            {"MonoTableInfo",    0x0C},
        };
        auto it = kSizes.find(key);
        return it != kSizes.end() ? it->second : 0;
    }

    bool validate(const uint8_t* headerBytes, size_t len) const override {
        if (len < 0x80) return false;
        return true;
    }
};

} // namespace omnibyte::dumper::unitymono
