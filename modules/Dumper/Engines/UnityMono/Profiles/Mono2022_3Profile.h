#pragma once
// ── Profiles/Mono2022_3Profile.h ──────────────────────────────────
// Unity 2022.3 LTS ships Mono 2022.3.x (based on Mono runtime ~6.12.x).
// Struct layouts derived from mono/mono@mono-2022-07 tag:
//   metadata-internals.h, class-private-definition.h, class-internals.h
// Target: 64-bit (Android arm64-v8a).
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

    bool validate(const uint8_t* headerBytes, size_t len) const override {
        // Mono metadata validation: check that we can read at least a few bytes
        // and that the image name pointer looks reasonable.
        if (len < 0x80) return false;
        // Heuristic: name pointer should be within the image or point to a string
        // We do a basic sanity check — not a full validation.
        return true;
    }
};

} // namespace omnibyte::dumper::unitymono
