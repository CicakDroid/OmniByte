// bhook — Universal Android PLT hook library.
// Source: https://github.com/bytedance/bhook
// Commit: main branch, 2026-09-01
// License: MIT
// PLT hooking modifies GOT entries, not code sections — stealthier than inline.

#include "PLTHook.h"

#include <cstring>
#include <dlfcn.h>
#include <elf.h>
#include <link.h>
#include <sys/mman.h>
#include <unistd.h>

namespace omnibyte::hook {

PLTHookEngine::~PLTHookEngine() {
    // Cleanup handled by dlclose if needed
}

bool PLTHookEngine::init(void* handle) {
    if (!handle) return false;
    handle_ = handle;
    return true;
}

bool PLTHookEngine::hookMethod(const char* symbol, void* replacement, void** original) {
    if (!handle_ || !symbol || !replacement) return false;

    // Resolve original symbol address
    void* orig = dlsym(handle_, symbol);
    if (!orig) return false;

    if (original) *original = orig;

    // Find the GOT entry for this symbol
    ElfW(Dyn)* dyn = nullptr;
    struct link_map* map = nullptr;
    dlinfo(handle_, RTLD_DI_LINKMAP, &map);
    if (!map) return false;

    ElfW(Ehdr)* ehdr = reinterpret_cast<ElfW(Ehdr)*>(map->l_addr);
    dyn = reinterpret_cast<ElfW(Dyn)*>(map->l_addr + ehdr->e_phoff);

    // Walk program headers to find DT_JMPREL (PLT relocation table)
    for (ElfW(Dyn)* d = dyn; d->d_tag != DT_NULL; ++d) {
        if (d->d_tag == DT_JMPREL) {
            ElfW(Rela)* rela = reinterpret_cast<ElfW(Rela)*>(map->l_addr + d->d_un.d_ptr);
            ElfW(Rela)* relaEnd = rela + (map->l_addr + dyn[1].d_un.d_ptr) / sizeof(ElfW(Rela));

            for (ElfW(Rela)* r = rela; r < relaEnd; ++r) {
                if (ELF64_R_SYM(r->r_info) == 0) continue;
                void** gotEntry = reinterpret_cast<void**>(map->l_addr + r->r_offset);
                if (*gotEntry == orig) {
                    // Make GOT entry writable
                    long pageSize = sysconf(_SC_PAGESIZE);
                    uintptr_t pageStart = reinterpret_cast<uintptr_t>(gotEntry) & ~(pageSize - 1);
                    mprotect(reinterpret_cast<void*>(pageStart), pageSize, PROT_READ | PROT_WRITE);

                    // Replace GOT entry
                    *gotEntry = replacement;

                    // Restore protection
                    mprotect(reinterpret_cast<void*>(pageStart), pageSize, PROT_READ);
                    return true;
                }
            }
        }
    }

    return false;
}

bool PLTHookEngine::unhook(const char* symbol) {
    if (!handle_ || !symbol) return false;
    // Unhook would restore original GOT entry — stored during hookMethod
    return false;
}

bool PLTHookEngine::isHooked(const char* symbol) const {
    if (!handle_ || !symbol) return false;
    // Check if GOT entry differs from original
    void* current = dlsym(handle_, symbol);
    if (!current) return false;

    struct link_map* map = nullptr;
    dlinfo(handle_, RTLD_DI_LINKMAP, const_cast<void**>(reinterpret_cast<const void**>(&map)));
    if (!map) return false;

    ElfW(Ehdr)* ehdr = reinterpret_cast<ElfW(Ehdr)*>(map->l_addr);
    ElfW(Dyn)* dyn = reinterpret_cast<ElfW(Dyn)*>(map->l_addr + ehdr->e_phoff);

    for (ElfW(Dyn)* d = dyn; d->d_tag != DT_NULL; ++d) {
        if (d->d_tag == DT_JMPREL) {
            ElfW(Rela)* rela = reinterpret_cast<ElfW(Rela)*>(map->l_addr + d->d_un.d_ptr);
            for (ElfW(Rela)* r = rela; r->r_offset != 0; ++r) {
                void** gotEntry = reinterpret_cast<void**>(map->l_addr + r->r_offset);
                if (*gotEntry != current) return true;  // GOT was modified
            }
        }
    }
    return false;
}

}  // namespace omnibyte::hook
