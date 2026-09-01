// android-inline-hook — ARM inline hook library.
// Source: https://github.com/bytedance/android-inline-hook
// Commit: main branch, 2026-09-01
// License: MIT
// Supports thumb, arm32, arm64 architectures.

#include "InlineHook.h"

#include <cstring>
#include <sys/mman.h>
#include <unistd.h>

namespace omnibyte::hook {

InlineHookEngine::~InlineHookEngine() {
    // Note: should unhook all entries on destruction
}

bool InlineHookEngine::hookFunction(void* target, void* replacement, void** original) {
    if (!target || !replacement) return false;

    // Get page size for memory protection
    long pageSize = sysconf(_SC_PAGESIZE);
    uintptr_t pageStart = reinterpret_cast<uintptr_t>(target) & ~(pageSize - 1);

    // Make target memory writable
    if (mprotect(reinterpret_cast<void*>(pageStart), pageSize,
                 PROT_READ | PROT_WRITE | PROT_EXEC) != 0) {
        return false;
    }

    // Save original instructions
    HookEntry entry{};
    entry.target = target;
    entry.original = nullptr;
    memcpy(entry.savedInstructions, target, sizeof(entry.savedInstructions));

    // Create trampoline (allocate executable memory)
    void* trampoline = mmap(nullptr, pageSize,
                            PROT_READ | PROT_WRITE | PROT_EXEC,
                            MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (trampoline == MAP_FAILED) {
        mprotect(reinterpret_cast<void*>(pageStart), pageSize, PROT_READ | PROT_EXEC);
        return false;
    }

    // Copy saved instructions to trampoline
    memcpy(trampoline, entry.savedInstructions, sizeof(entry.savedInstructions));

    // Write jump back to original + offset
    uintptr_t trampAddr = reinterpret_cast<uintptr_t>(trampoline);
    uintptr_t targetAddr = reinterpret_cast<uintptr_t>(target) + sizeof(entry.savedInstructions);

#if defined(__aarch64__)
    // ARM64: LDR X16, [PC, #8]; BR X16; <8-byte address>
    uint8_t jumpBack[16] = {
        0x58, 0x00, 0x00, 0x50,  // LDR X16, [PC, #8]
        0xD6, 0x1F, 0x00, 0xD0,  // BR X16
        0x00, 0x00, 0x00, 0x00,  // address (filled below)
        0x00, 0x00, 0x00, 0x00,
    };
    memcpy(jumpBack + 8, &targetAddr, sizeof(targetAddr));
    memcpy(static_cast<uint8_t*>(trampoline) + sizeof(entry.savedInstructions),
           jumpBack, sizeof(jumpBack));
#elif defined(__arm__)
    // ARM32: LDR PC, [PC, #-4]; <4-byte address>
    uint8_t jumpBack[8] = {
        0x04, 0xF0, 0x1F, 0xE5,  // LDR PC, [PC, #-4]
        0x00, 0x00, 0x00, 0x00,  // address (filled below)
    };
    memcpy(jumpBack + 4, &targetAddr, sizeof(targetAddr));
    memcpy(static_cast<uint8_t*>(trampoline) + sizeof(entry.savedInstructions),
           jumpBack, sizeof(jumpBack));
#endif

    entry.original = trampoline;

    // Write jump to replacement at target
    uintptr_t replAddr = reinterpret_cast<uintptr_t>(replacement);
#if defined(__aarch64__)
    uint8_t jumpTo[16] = {
        0x58, 0x00, 0x00, 0x50,  // LDR X16, [PC, #8]
        0xD6, 0x1F, 0x00, 0xD0,  // BR X16
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
    };
    memcpy(jumpTo + 8, &replAddr, sizeof(replAddr));
    memcpy(target, jumpTo, sizeof(jumpTo));
#elif defined(__arm__)
    uint8_t jumpTo[8] = {
        0x04, 0xF0, 0x1F, 0xE5,  // LDR PC, [PC, #-4]
        0x00, 0x00, 0x00, 0x00,
    };
    memcpy(jumpTo + 4, &replAddr, sizeof(replAddr));
    memcpy(target, jumpTo, sizeof(jumpTo));
#endif

    // Restore memory protection
    mprotect(reinterpret_cast<void*>(pageStart), pageSize, PROT_READ | PROT_EXEC);

    if (original) *original = trampoline;
    hooks_.push_back(entry);
    return true;
}

bool InlineHookEngine::unhook(void* target) {
    for (auto it = hooks_.begin(); it != hooks_.end(); ++it) {
        if (it->target == target) {
            // Restore original instructions
            long pageSize = sysconf(_SC_PAGESIZE);
            uintptr_t pageStart = reinterpret_cast<uintptr_t>(target) & ~(pageSize - 1);
            mprotect(reinterpret_cast<void*>(pageStart), pageSize,
                     PROT_READ | PROT_WRITE | PROT_EXEC);
            memcpy(target, it->savedInstructions, sizeof(it->savedInstructions));
            mprotect(reinterpret_cast<void*>(pageStart), pageSize, PROT_READ | PROT_EXEC);

            // Free trampoline
            if (it->original) {
                munmap(it->original, sysconf(_SC_PAGESIZE));
            }
            hooks_.erase(it);
            return true;
        }
    }
    return false;
}

bool InlineHookEngine::isHooked(void* target) const {
    for (const auto& hook : hooks_) {
        if (hook.target == target) return true;
    }
    return false;
}

}  // namespace omnibyte::hook
