// NativePmsHook — inline hook on libandroid_runtime.so to intercept PMS native calls.
// Patches JNI_RegisterNatives or getPackageInfo to return spoofed PackageInfo.

#include "NativePmsHook.h"

#include <cstdio>
#include <cstring>
#include <cstdint>
#include <dlfcn.h>
#include <sys/mman.h>
#include <unistd.h>

namespace omnibyte::runtime::backends {

void* NativePmsHook::hookHandler_ = nullptr;

// --- Lifecycle ---

bool NativePmsHook::initialize() {
    if (initialized_) return true;

    if (!findModule("libandroid_runtime.so")) return false;

    // Try to find getPackageInfo or RegisterNatives.
    targetFunc_ = findSymbol("android_util_PackageManager");
    if (!targetFunc_) {
        // Fallback: search for RegisterNatives which is the JNI entry point.
        targetFunc_ = dlsym(RTLD_DEFAULT, "RegisterNatives");
    }

    if (!targetFunc_) return false;

    initialized_ = true;
    return true;
}

bool NativePmsHook::hook() {
    if (!initialized_ || hooked_) return hooked_;

    // Save original bytes.
    memcpy(originalBytes_, targetFunc_, sizeof(originalBytes_));

    // Calculate patch length — need to overwrite enough for a branch instruction.
    // arm64: brk #0 is 4 bytes, or we use an absolute branch (ldr+br = 16 bytes).
    patchLen_ = 16; // ldr x16, [pc, #8]; br x16; <address>

    if (!makeWritable(targetFunc_, patchLen_)) return false;

#if defined(__aarch64__)
    // arm64: absolute branch via x16 scratch register.
    // ldr x16, [pc, #8]  → 0x58000050
    // br  x16            → 0xD61F0200
    // <8-byte address>
    uint8_t patch[16];
    uint32_t ldr_x16 = 0x58000050; // ldr x16, [pc, #8]
    uint32_t br_x16  = 0xD61F0200; // br x16
    memcpy(patch, &ldr_x16, 4);
    memcpy(patch + 4, &br_x16, 4);
    uintptr_t handlerAddr = reinterpret_cast<uintptr_t>(hookHandler_);
    memcpy(patch + 8, &handlerAddr, 8);
    memcpy(targetFunc_, patch, patchLen_);
#elif defined(__arm__)
    // arm32: absolute branch via pc-relative or ldr pc.
    // ldr pc, [pc, #-4]  → 0xE51FF004
    // <4-byte address>
    uint8_t patch[8];
    uint32_t ldr_pc = 0xE51FF004; // ldr pc, [pc, #-4]
    memcpy(patch, &ldr_pc, 4);
    uintptr_t handlerAddr = reinterpret_cast<uintptr_t>(hookHandler_);
    memcpy(patch + 4, &handlerAddr, 4);
    patchLen_ = 8;
    memcpy(targetFunc_, patch, patchLen_);
#else
    // Unsupported arch — fail gracefully.
    mprotect(targetFunc_, patchLen_, PROT_READ | PROT_EXEC);
    return false;
#endif

    hooked_ = true;
    return true;
}

bool NativePmsHook::unhook() {
    if (!hooked_ || !targetFunc_) return true;

    if (!makeWritable(targetFunc_, patchLen_)) return false;

    memcpy(targetFunc_, originalBytes_, patchLen_);

    // Restore original protection.
    mprotect(targetFunc_, patchLen_, PROT_READ | PROT_EXEC);

    hooked_ = false;
    return true;
}

// --- Internal ---

bool NativePmsHook::findModule(const char* moduleName) {
    FILE* maps = fopen("/proc/self/maps", "r");
    if (!maps) return false;

    char line[512];
    while (fgets(line, sizeof(line), maps)) {
        if (!strstr(line, moduleName)) continue;

        // Parse: addr-addr perms offset dev inode pathname
        uintptr_t start = 0, end = 0;
        if (sscanf(line, "%lx-%lx", &start, &end) == 2) {
            moduleBase_ = start;
            moduleSize_ = end - start;
            fclose(maps);
            return true;
        }
    }

    fclose(maps);
    return false;
}

void* NativePmsHook::findSymbol(const char* symbolName) {
    // Try dlsym on the loaded module handle.
    void* handle = dlopen("libandroid_runtime.so", RTLD_NOW | RTLD_NOLOAD);
    if (handle) {
        void* sym = dlsym(handle, symbolName);
        dlclose(handle);
        if (sym) return sym;
    }

    // Manual search: scan module memory for symbol.
    // Parse ELF symbol table from module base.
    if (!moduleBase_ || moduleSize_ == 0) return nullptr;

    auto* base = reinterpret_cast<uint8_t*>(moduleBase_);

    // ELF header check.
    if (base[0] != 0x7f || base[1] != 'E' || base[2] != 'L' || base[3] != 'F') {
        return nullptr;
    }

#if defined(__aarch64__)
    auto* ehdr = reinterpret_cast<Elf64_Ehdr*>(base);
    auto* shdr = reinterpret_cast<Elf64_Shdr*>(base + ehdr->e_shoff);

    // Find .dynsym section.
    for (int i = 0; i < ehdr->e_shnum; ++i) {
        if (shdr[i].sh_type == SHT_DYNSYM) {
            auto* symtab = reinterpret_cast<Elf64_Sym*>(base + shdr[i].sh_offset);
            auto* strtab = reinterpret_cast<const char*>(
                base + shdr[shdr[i].sh_link].sh_offset);
            size_t count = shdr[i].sh_size / sizeof(Elf64_Sym);

            for (size_t j = 0; j < count; ++j) {
                if (strcmp(strtab + symtab[j].st_name, symbolName) == 0) {
                    return base + symtab[j].st_value;
                }
            }
        }
    }
#elif defined(__arm__)
    auto* ehdr = reinterpret_cast<Elf32_Ehdr*>(base);
    auto* shdr = reinterpret_cast<Elf32_Shdr*>(base + ehdr->e_shoff);

    for (int i = 0; i < ehdr->e_shnum; ++i) {
        if (shdr[i].sh_type == SHT_DYNSYM) {
            auto* symtab = reinterpret_cast<Elf32_Sym*>(base + shdr[i].sh_offset);
            auto* strtab = reinterpret_cast<const char*>(
                base + shdr[shdr[i].sh_link].sh_offset);
            size_t count = shdr[i].sh_size / sizeof(Elf32_Sym);

            for (size_t j = 0; j < count; ++j) {
                if (strcmp(strtab + symtab[j].st_name, symbolName) == 0) {
                    return base + symtab[j].st_value;
                }
            }
        }
    }
#endif

    return nullptr;
}

bool NativePmsHook::patchInstructions(void* target, const void* hook, size_t len) {
    if (!makeWritable(target, len)) return false;
    memcpy(target, hook, len);
    return true;
}

bool NativePmsHook::makeWritable(void* addr, size_t len) {
    long pageSize = sysconf(_SC_PAGESIZE);
    auto* page = reinterpret_cast<void*>(
        reinterpret_cast<uintptr_t>(addr) & ~(pageSize - 1));
    size_t protectLen = len + (reinterpret_cast<uintptr_t>(addr) - reinterpret_cast<uintptr_t>(page));

    return mprotect(page, protectLen, PROT_READ | PROT_WRITE | PROT_EXEC) == 0;
}

} // namespace omnibyte::runtime::backends
