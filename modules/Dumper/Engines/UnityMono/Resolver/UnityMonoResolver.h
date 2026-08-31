#pragma once
// UnityMono — runtime symbol resolver for Mono metadata.
// Resolves mono_get_root_domain, domain assemblies, and walks
// live MonoImage → MonoClass → MonoMethod via process memory.
#include "../../../DumperCore/IDumperEngine.h"
#include "../../../DumperCore/IEngineProfile.h"
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>

namespace omnibyte::dumper::unitymono {

class UnityMonoResolver {
public:
    static DumpResult resolveSymbols(const AnalysisTarget& target,
                                     const std::shared_ptr<IEngineProfile>& profile) {
        DumpResult result;
        result.engineName = "Unity Mono";
        result.detectedVersion = profile ? profile->version() : "unknown";

        if (!profile) {
            result.errorMessage = "No profile provided";
            return result;
        }

        if (target.isFile()) {
            result.errorMessage = "Resolver requires live process target (use analyzer for file targets)";
            return result;
        }

        return resolveFromProcess(target, profile);
    }

private:
    static bool readProcessMemory(int pid, uintptr_t address, void* out, size_t size) {
        char path[64];
        snprintf(path, sizeof(path), "/proc/%d/mem", pid);
        FILE* f = fopen(path, "rb");
        if (!f) return false;
        if (fseek(f, static_cast<long>(address), SEEK_SET) != 0) {
            fclose(f);
            return false;
        }
        size_t read = fread(out, 1, size, f);
        fclose(f);
        return read == size;
    }

    template<typename T>
    static bool readProcessValue(int pid, uintptr_t address, T& out) {
        return readProcessMemory(pid, address, &out, sizeof(T));
    }

    static std::string readProcessString(int pid, uintptr_t address, size_t maxLen = 256) {
        std::vector<char> buf(maxLen);
        if (!readProcessMemory(pid, address, buf.data(), maxLen)) return "";
        buf[maxLen - 1] = '\0';
        return std::string(buf.data());
    }

    static uint64_t readProcessPtr(int pid, uintptr_t address) {
        uint64_t val = 0;
        readProcessValue(pid, address, val);
        return val;
    }

    static uint32_t readProcessU32(int pid, uintptr_t address) {
        uint32_t val = 0;
        readProcessValue(pid, address, val);
        return val;
    }

    static uint16_t readProcessU16(int pid, uintptr_t address) {
        uint16_t val = 0;
        readProcessValue(pid, address, val);
        return val;
    }

    static uint8_t readProcessU8(int pid, uintptr_t address) {
        uint8_t val = 0;
        readProcessValue(pid, address, val);
        return val;
    }

    struct MonoImageFields {
        uint64_t raw_data = 0;
        uint32_t raw_data_len = 0;
        uint64_t name = 0;
        uint64_t assembly_name = 0;
        uint64_t tables_base = 0;
    };

    static MonoImageFields readMonoImageFields(int pid, uintptr_t imageAddr,
                                                const std::shared_ptr<IEngineProfile>& profile) {
        MonoImageFields f;
        f.raw_data = readProcessPtr(pid, imageAddr + profile->offsetOf("MonoImage_raw_data"));
        f.raw_data_len = readProcessU32(pid, imageAddr + profile->offsetOf("MonoImage_raw_data_len"));
        f.name = readProcessPtr(pid, imageAddr + profile->offsetOf("MonoImage_name"));
        f.assembly_name = readProcessPtr(pid, imageAddr + profile->offsetOf("MonoImage_assembly_name"));
        return f;
    }

    struct MonoClassFields {
        uint64_t name = 0;
        uint64_t name_space = 0;
        uint64_t image = 0;
        uint32_t type_token = 0;
        int instance_size = 0;
        uint64_t fields = 0;
        uint64_t methods = 0;
        uint32_t field_first = 0;
        uint32_t field_count = 0;
        uint32_t method_first = 0;
        uint32_t method_count = 0;
    };

    static MonoClassFields readMonoClassFields(int pid, uintptr_t classAddr,
                                                const std::shared_ptr<IEngineProfile>& profile) {
        MonoClassFields c;
        c.name = readProcessPtr(pid, classAddr + profile->offsetOf("MonoClass_name"));
        c.name_space = readProcessPtr(pid, classAddr + profile->offsetOf("MonoClass_name_space"));
        c.image = readProcessPtr(pid, classAddr + profile->offsetOf("MonoClass_image"));
        c.type_token = readProcessU32(pid, classAddr + profile->offsetOf("MonoClass_type_token"));
        c.instance_size = static_cast<int>(readProcessU32(pid, classAddr + profile->offsetOf("MonoClass_instance_size")));
        c.fields = readProcessPtr(pid, classAddr + profile->offsetOf("MonoClass_fields"));
        c.methods = readProcessPtr(pid, classAddr + profile->offsetOf("MonoClass_methods"));
        c.field_first = readProcessU32(pid, classAddr + profile->offsetOf("MonoClass_field_first"));
        c.field_count = readProcessU32(pid, classAddr + profile->offsetOf("MonoClass_field_count"));
        c.method_first = readProcessU32(pid, classAddr + profile->offsetOf("MonoClass_method_first"));
        c.method_count = readProcessU32(pid, classAddr + profile->offsetOf("MonoClass_method_count"));
        return c;
    }

    // MonoClassField: type(8) + name(8) + parent(8) + offset(4) = 0x20
    struct MonoClassFieldInfo {
        uint64_t type = 0;
        uint64_t name = 0;
        uint64_t parent = 0;
        int32_t offset = 0;
    };

    static MonoClassFieldInfo readMonoClassField(int pid, uintptr_t fieldAddr,
                                                  const std::shared_ptr<IEngineProfile>& profile) {
        MonoClassFieldInfo f;
        f.type = readProcessPtr(pid, fieldAddr + profile->offsetOf("MonoClassField_type"));
        f.name = readProcessPtr(pid, fieldAddr + profile->offsetOf("MonoClassField_name"));
        f.parent = readProcessPtr(pid, fieldAddr + profile->offsetOf("MonoClassField_parent"));
        f.offset = static_cast<int32_t>(readProcessU32(pid, fieldAddr + profile->offsetOf("MonoClassField_offset")));
        return f;
    }

    // MonoMethod: flags(2) + iflags(2) + token(4) + klass(8) + signature(8) + name(8) = 0x20
    struct MonoMethodInfo {
        uint16_t flags = 0;
        uint32_t token = 0;
        uint64_t klass = 0;
        uint64_t name = 0;
    };

    static MonoMethodInfo readMonoMethod(int pid, uintptr_t methodAddr,
                                          const std::shared_ptr<IEngineProfile>& profile) {
        MonoMethodInfo m;
        m.flags = readProcessU16(pid, methodAddr + profile->offsetOf("MonoMethod_flags"));
        m.token = readProcessU32(pid, methodAddr + profile->offsetOf("MonoMethod_token"));
        m.klass = readProcessPtr(pid, methodAddr + profile->offsetOf("MonoMethod_klass"));
        m.name = readProcessPtr(pid, methodAddr + profile->offsetOf("MonoMethod_name"));
        return m;
    }

    // MonoType: data(8) + attrs(2) + type_enum(1) + mods(1) = 12 bytes
    static std::string resolveTypeName(int pid, uintptr_t typeAddr) {
        if (typeAddr == 0) return "unknown";
        uint8_t typeEnum = readProcessU8(pid, typeAddr + 10);
        switch (typeEnum) {
            case 0x01: return "void";
            case 0x02: return "bool";
            case 0x03: return "char";
            case 0x04: return "int8_t";
            case 0x05: return "uint8_t";
            case 0x06: return "int16_t";
            case 0x07: return "uint16_t";
            case 0x08: return "int32_t";
            case 0x09: return "uint32_t";
            case 0x0A: return "int64_t";
            case 0x0B: return "uint64_t";
            case 0x0C: return "float";
            case 0x0D: return "double";
            case 0x0E: return "string";
            case 0x12: return "class";
            case 0x1C: return "object";
            default:   return "type_0x" + std::to_string(typeEnum);
        }
    }

    static DumpResult resolveFromProcess(const AnalysisTarget& target,
                                         const std::shared_ptr<IEngineProfile>& profile) {
        DumpResult result;
        result.engineName = "Unity Mono";
        result.detectedVersion = profile->version();

        int pid = target.pid;
        uintptr_t base = target.baseAddress;

        if (pid <= 0 || base == 0) {
            result.errorMessage = "Invalid process target (pid=" + std::to_string(pid) +
                                  ", base=0x" + std::to_string(base) + ")";
            return result;
        }

        // Attempt to read MonoImage at the base address.
        // In a live process, the caller should have already located
        // the MonoImage pointer (e.g. via mono_get_root_domain →
        // domain->domains → assemblies → image). For now, we attempt
        // to parse from base as if it points to a MonoImage.
        MonoImageFields img = readMonoImageFields(pid, base, profile);

        if (img.raw_data == 0 || img.raw_data_len == 0) {
            result.errorMessage = "Cannot read MonoImage at base address 0x" +
                                  std::to_string(base) + " — raw_data is null";
            return result;
        }

        result.setMeta("mono_image_name", readProcessString(pid, img.name));
        result.setMeta("mono_assembly_name", readProcessString(pid, img.assembly_name));
        result.setMeta("raw_data_va", "0x" + std::to_string(img.raw_data));
        result.setMeta("raw_data_len", std::to_string(img.raw_data_len));

        // Read metadata tables from the process
        uintptr_t tablesBase = base + profile->offsetOf("MonoImage_tables");

        // TypeDef table (index 0x02)
        uintptr_t typeDefInfoAddr = tablesBase + (0x02 * 16);
        uint32_t typeDefRows = readProcessU32(pid, typeDefInfoAddr);
        uint32_t typeDefRowSize = readProcessU32(pid, typeDefInfoAddr + 4);
        uint64_t typeDefData = readProcessPtr(pid, typeDefInfoAddr + 8);

        // FieldDef table (index 0x04)
        uintptr_t fieldDefInfoAddr = tablesBase + (0x04 * 16);
        uint32_t fieldDefRows = readProcessU32(pid, fieldDefInfoAddr);
        uint32_t fieldDefRowSize = readProcessU32(pid, fieldDefInfoAddr + 4);

        // MethodDef table (index 0x06)
        uintptr_t methodDefInfoAddr = tablesBase + (0x06 * 16);
        uint32_t methodDefRows = readProcessU32(pid, methodDefInfoAddr);
        uint32_t methodDefRowSize = readProcessU32(pid, methodDefInfoAddr + 4);

        // Read string heap header for string resolution
        uintptr_t stringHeapAddr = base + profile->offsetOf("MonoImage_heap_strings");
        uint32_t stringHeapSize = readProcessU32(pid, stringHeapAddr);

        bool stringIsLong = (stringHeapSize > 0xFFFF);
        bool tableIsLong = (fieldDefRows > 0xFFFF || methodDefRows > 0xFFFF);

        // Parse TypeDef rows from process memory
        for (uint32_t i = 0; i < typeDefRows; ++i) {
            uintptr_t rowAddr = typeDefData + (i * typeDefRowSize);

            // flags(4) + name_idx + ns_idx + extends + field_list + method_list
            uint32_t flags = readProcessU32(pid, rowAddr);
            size_t off = rowAddr + 4;

            uint32_t nameIdx = stringIsLong ? readProcessU32(pid, off) : readProcessU16(pid, off);
            off += stringIsLong ? 4 : 2;
            uint32_t nsIdx = stringIsLong ? readProcessU32(pid, off) : readProcessU16(pid, off);
            off += stringIsLong ? 4 : 2;
            uint32_t extendsIdx = tableIsLong ? readProcessU32(pid, off) : readProcessU16(pid, off);
            off += tableIsLong ? 4 : 2;
            uint32_t fieldList = tableIsLong ? readProcessU32(pid, off) : readProcessU16(pid, off);
            off += tableIsLong ? 4 : 2;
            uint32_t methodList = tableIsLong ? readProcessU32(pid, off) : readProcessU16(pid, off);

            // Read name from string heap
            uintptr_t stringHeapBase = base + profile->offsetOf("MonoImage_heap_strings") + 8;
            std::string typeName = readProcessString(pid, stringHeapBase + nameIdx);
            std::string typeNs = readProcessString(pid, stringHeapBase + nsIdx);

            if (typeName.empty()) continue;
            std::string fullName = typeNs.empty() ? typeName : (typeNs + "." + typeName);

            TypeEntry te;
            te.name = fullName;
            te.typeId = i;
            te.address = 0;
            result.typeTable.push_back(te);

            // Parse fields
            uint32_t fieldEnd = (i + 1 < typeDefRows) ? fieldList : (fieldDefRows + 1);
            for (uint32_t fi = fieldList; fi < fieldEnd && fi > 0; ++fi) {
                uint32_t fieldIdx = fi - 1;
                if (fieldIdx >= fieldDefRows) break;
                uintptr_t fAddr = typeDefData + (fieldIdx * fieldDefRowSize);

                uint16_t fFlags = readProcessU16(pid, fAddr);
                size_t fOff = fAddr + 2;
                uint32_t fNameIdx = stringIsLong ? readProcessU32(pid, fOff) : readProcessU16(pid, fOff);

                std::string fieldName = readProcessString(pid, stringHeapBase + fNameIdx);

                FieldEntry fe;
                fe.name = fieldName;
                fe.declaringType = fullName;
                fe.typeName = ""; // resolved via MonoType at runtime
                fe.offset = 0;
                result.fieldTable.push_back(fe);
            }

            // Parse methods
            uint32_t methodEnd = (i + 1 < typeDefRows) ? methodList : (methodDefRows + 1);
            for (uint32_t mi = methodList; mi < methodEnd && mi > 0; ++mi) {
                uint32_t methodIdx = mi - 1;
                if (methodIdx >= methodDefRows) break;
                uintptr_t mAddr = typeDefData + (methodIdx * methodDefRowSize);

                uint32_t rva = readProcessU32(pid, mAddr);
                uint16_t mImplFlags = readProcessU16(pid, mAddr + 4);
                uint16_t mFlags = readProcessU16(pid, mAddr + 6);
                size_t mOff = mAddr + 8;
                uint32_t mNameIdx = stringIsLong ? readProcessU32(pid, mOff) : readProcessU16(pid, mOff);

                std::string methodName = readProcessString(pid, stringHeapBase + mNameIdx);

                MethodEntry me;
                me.name = methodName;
                me.declaringType = fullName;
                me.methodIndex = mi - methodList;
                me.address = 0; // RVA-based; actual address requires JIT info
                me.isStatic = (mFlags & 0x0010) != 0;
                me.isVirtual = (mFlags & 0x0040) != 0;
                result.methodTable.push_back(me);
            }
        }

        result.success = !result.typeTable.empty();
        if (!result.success) {
            result.errorMessage = "No types resolved from process memory";
        }

        return result;
    }
};

} // namespace omnibyte::dumper::unitymono
