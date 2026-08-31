#pragma once
// ── Profiles/Mono6000_4Profile.h ──────────────────────────────────
// Unity 6000.4 ships Mono ~6000.4.x (based on .NET 6 runtime).
// Struct layouts derived from mono/mono@mono-6000.4 tag:
//   metadata-internals.h, class-private-definition.h, class-internals.h
// Target: 64-bit (Android arm64-v8a).
//
// Core structs stable. This is the latest Mono version in the 6000.x series.
// Minor additions to MonoImage for improved debugging support.
#include "../../../DumperCore/IEngineProfile.h"
#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>

namespace omnibyte::dumper::unitymono {

class Mono6000_4Profile : public IEngineProfile {
public:
    std::string version() const override { return "6000.4"; }

    uint64_t offsetOf(const std::string& key) const override {
        static const std::unordered_map<std::string, uint64_t> kOffsets = {
            // ── MonoImage (metadata-internals.h) ────────────────────
            {"MonoImageOffset",                 0x0},
            {"MonoImage_sizeof",                0x3C0},

            {"MonoImage_raw_data",              0x10},
            {"MonoImage_raw_data_len",          0x18},

            {"MonoImage_name",                  0x30},
            {"MonoImage_filename",              0x38},
            {"MonoImage_assembly_name",         0x40},
            {"MonoImage_module_name",           0x48},
            {"MonoImage_version",               0x58},
            {"MonoImage_guid",                  0x68},

            {"MonoImage_md_version_major",      0x70},
            {"MonoImage_md_version_minor",      0x72},

            {"MonoImage_heap_strings",          0x88},
            {"MonoImage_heap_us",               0x90},
            {"MonoImage_heap_blob",             0x98},
            {"MonoImage_heap_guid",             0xA0},
            {"MonoImage_heap_tables",           0xA8},

            {"MonoImage_tables_base",           0xB0},
            {"MonoImage_tables",                0xB8},
            {"MonoTableInfo_sizeof",            0x10},

            {"MonoImage_assembly",              0x348},

            // ── MonoClass (class-private-definition.h) ──────────────
            {"MonoClass_sizeof",                0x118},

            {"MonoClass_element_class",         0x00},
            {"MonoClass_cast_class",            0x08},
            {"MonoClass_supertypes",            0x10},
            {"MonoClass_idepth",                0x18},
            {"MonoClass_rank",                  0x1A},
            {"MonoClass_class_kind",            0x1B},
            {"MonoClass_instance_size",         0x1C},

            {"MonoClass_bitfield1",             0x20},
            {"MonoClass_min_align",             0x24},
            {"MonoClass_bitfield2",             0x25},
            {"MonoClass_bitfield3",             0x26},
            {"MonoClass_bitfield4",             0x27},

            {"MonoClass_parent",                0x28},
            {"MonoClass_nested_in",             0x30},
            {"MonoClass_image",                 0x38},
            {"MonoClass_name",                  0x40},
            {"MonoClass_name_space",            0x48},

            {"MonoClass_type_token",            0x50},
            {"MonoClass_vtable_size",           0x54},

            {"MonoClass_interface_count",       0x58},
            {"MonoClass_interface_id",          0x5C},
            {"MonoClass_max_interface_id",      0x60},
            {"MonoClass_interface_offsets_count", 0x64},
            {"MonoClass_interfaces_packed",     0x68},
            {"MonoClass_interface_offsets_packed", 0x70},
            {"MonoClass_interface_bitmap",      0x78},
            {"MonoClass_interfaces",            0x80},

            {"MonoClass_sizes",                 0x88},
            {"MonoClass_flags",                 0x8C},
            {"MonoClass_field_first",           0x90},
            {"MonoClass_field_count",           0x94},
            {"MonoClass_method_first",          0x98},
            {"MonoClass_method_count",          0x9C},

            {"MonoClass_ref_info_handle",       0xA0},
            {"MonoClass_marshal_info",          0xA8},
            {"MonoClass_fields",                0xB0},
            {"MonoClass_methods",               0xB8},

            {"MonoClass_this_arg",              0xC0},
            {"MonoClass__byval_arg",            0xD0},

            {"MonoClass_generic_class",         0xE0},
            {"MonoClass_generic_container",     0xE8},
            {"MonoClass_gc_descr",              0xF0},
            {"MonoClass_runtime_info",          0xF8},
            {"MonoClass_next_class_cache",      0x100},
            {"MonoClass_vtable",                0x108},
            {"MonoClass_ext",                   0x110},

            // ── MonoClassField (class-internals.h) ──────────────────
            {"MonoClassField_sizeof",           0x20},

            {"MonoClassField_type",             0x00},
            {"MonoClassField_name",             0x08},
            {"MonoClassField_parent",           0x10},
            {"MonoClassField_offset",           0x18},

            // ── MonoMethod (class-internals.h) ──────────────────────
            {"MonoMethod_sizeof",               0x28},

            {"MonoMethod_flags",                0x00},
            {"MonoMethod_iflags",               0x02},
            {"MonoMethod_token",                0x04},
            {"MonoMethod_klass",                0x08},
            {"MonoMethod_signature",            0x10},
            {"MonoMethod_name",                 0x18},
            {"MonoMethod_bitfields",            0x20},
            {"MonoMethod_slot",                 0x24},

            // ── MonoType (class-internals.h) ────────────────────────
            {"MonoType_sizeof",                 0x10},

            {"MonoType_data",                   0x00},
            {"MonoType_attrs",                  0x08},
            {"MonoType_type",                   0x0A},
            {"MonoType_num_mods",               0x0B},
            {"MonoType_byref",                  0x0B},
            {"MonoType_pinned",                 0x0B},
        };
        auto it = kOffsets.find(key);
        return it != kOffsets.end() ? it->second : 0;
    }

    size_t structSize(const std::string& key) const override {
        static const std::unordered_map<std::string, size_t> kSizes = {
            {"MonoImage",      0x3C0},
            {"MonoClass",      0x118},
            {"MonoClassField", 0x20},
            {"MonoMethod",     0x28},
            {"MonoType",       0x10},
            {"MonoStreamHeader", 0x08},
            {"MonoTableInfo",  0x10},
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
