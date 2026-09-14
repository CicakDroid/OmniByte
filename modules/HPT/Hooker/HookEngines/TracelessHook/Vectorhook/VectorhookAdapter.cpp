/**
 * @file VectorhookAdapter.cpp
 * @brief Stealth KPM inline-hook adapter implementation.
 *
 *  Engine : shpte whole-page DBI clone via no-superkey sysinfo bridge (arm64 only)
 *  Commit : v0.1.0 (2026-09-05)
 *  License: MIT (stealth-core); Vector itself is GPL-3.0 but this adapter wraps only the MIT layer
 *  Source : https://github.com/1013503897/stealth-core
 *  Refs   : demo shows kpm_hook_force_enable() → kpm_hook_init() →
 *           kpm_inline_hooker(target, hooker) → backup → kpm_inline_unhooker() → kpm_hook_shutdown()
 *
 *  Startup flow (mirrors demohook.c):
 *      kpm_hook_force_enable()   — bypass Vector-only process gate (we ARE the injected agent)
 *      kpm_hook_init()           — probe the KPM bridge; caches getpid()
 *      kpm_inline_hooker()       — clone target's code page, override entry -> hooker; backup = original
 *      kpm_inline_unhooker()     — drop the entry override; target reverts to clone copy
 *      kpm_hook_shutdown()       — release all clone mappings
 *
 *  Anti-tamper stack (stealth-core built-in, no extra work here):
 *      CRC on target .text before+after → detect re-hook attempts
 *      /proc/maps scan spoof → defeat hidden-overlayfs mount detection
 *      ptrace Self-Ptrace-Poison → PTRACE_TRACEME on self defeats debugger attach
 *      VMA-less ghost clone path → clone lives in KPM kernel memory, invisible to /proc/maps
 */

#include "VectorhookAdapter.h"

#include <android/log.h>
#include <cstring>
#include <dlfcn.h>

#define TAG "VectorhookAdapter"

namespace omnibyte::runtime::backends {

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

VectorhookAdapter::VectorhookAdapter()
    : m_initialised(false)
    , m_available(false)
{
    // Probe once: can we resolve all kpmhook symbols?
    // The KPM bridge must be armed by shctl before this point.
    m_available = init();
}

VectorhookAdapter::~VectorhookAdapter() {
    if (m_initialised) {
        kpm_hook_shutdown();
        __android_log_print(ANDROID_LOG_INFO, TAG, "KPM hook engine shut down");
    }
}

// ---------------------------------------------------------------------------
// IHookBackend interface
// ---------------------------------------------------------------------------

std::string VectorhookAdapter::name() const {
    return "Vectorhook";
}

bool VectorhookAdapter::isAvailable() const {
    return m_available;
}

bool VectorhookAdapter::hookFunction(uintptr_t address, void* hookFn, void** origFn) {
    if (!m_initialised) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "hookFunction: not initialised");
        return false;
    }
    if (!address || !hookFn) {
        __android_log_print(ANDROID_LOG_WARN, TAG, "hookFunction: invalid args (addr=%p hook=%p)",
                            reinterpret_cast<void*>(address), hookFn);
        return false;
    }

    void* backup = kpm_inline_hooker(reinterpret_cast<void*>(address), hookFn);
    if (!backup) {
        __android_log_print(ANDROID_LOG_WARN, TAG, "hookFunction: KPM hook rejected at %p "
                            "(bridge armed but hook failed — function may span page boundary)",
                            reinterpret_cast<void*>(address));
        return false;
    }

    if (origFn) *origFn = backup;

    __android_log_print(ANDROID_LOG_INFO, TAG, "hookFunction: hooked %p -> %p (backup=%p)",
                        reinterpret_cast<void*>(address), hookFn, backup);
    return true;
}

bool VectorhookAdapter::unhook(uintptr_t address) {
    if (!m_initialised) return false;
    if (!address) return false;

    int ok = kpm_inline_unhooker(reinterpret_cast<void*>(address));
    if (!ok) {
        __android_log_print(ANDROID_LOG_WARN, TAG, "unhook: KPM unhook failed at %p "
                            "(not a KPM-hooked function, or already unhooked)",
                            reinterpret_cast<void*>(address));
        return false;
    }

    __android_log_print(ANDROID_LOG_INFO, TAG, "unhook: removed hook at %p",
                        reinterpret_cast<void*>(address));
    return true;
}

bool VectorhookAdapter::patchMemory(uintptr_t address, const uint8_t* data, size_t size) {
    // KPM whole-page clone does not expose a memory-patch API.
    // Inline hooking is the intended interface; direct .text writes would break
    // the CRC anti-tamper and defeat the traceless design.
    __android_log_print(ANDROID_LOG_WARN, TAG,
        "patchMemory: not supported by Vectorhook engine (traceless design forbids .text writes)");
    return false;
}

// ---------------------------------------------------------------------------
// Private
// ---------------------------------------------------------------------------

bool VectorhookAdapter::init() {
    // Resolve the kpmhook symbols from the injected native layer.
    // stealth-core's libkpmhook is linked into Vector's native lib.
    // If the symbols aren't present (standalone build, no KPM), bail.
    auto force_enable = reinterpret_cast<void(*)()>(
        dlsym(RTLD_DEFAULT, "kpm_hook_force_enable"));
    auto hook_init = reinterpret_cast<int(*)()>(
        dlsym(RTLD_DEFAULT, "kpm_hook_init"));
    auto inline_hooker = reinterpret_cast<void*(*)(void*, void*)>(
        dlsym(RTLD_DEFAULT, "kpm_inline_hooker"));
    auto inline_unhooker = reinterpret_cast<int(*)(void*)>(
        dlsym(RTLD_DEFAULT, "kpm_inline_unhooker"));
    auto hook_shutdown = reinterpret_cast<void(*)()>(
        dlsym(RTLD_DEFAULT, "kpm_hook_shutdown"));

    if (!force_enable || !hook_init || !inline_hooker || !inline_unhooker || !hook_shutdown) {
        __android_log_print(ANDROID_LOG_WARN, TAG,
            "init: kpmhook symbols not found (stealth-core not linked or KPM not loaded). "
            "Hooking unavailable.");
        return false;
    }

    // Bypass the Vector-only process gate (we ARE the injected agent)
    force_enable();

    // Probe the bridge; if shpte KPM isn't loaded this returns non-zero
    int rc = hook_init();
    if (rc != 0) {
        __android_log_print(ANDROID_LOG_WARN, TAG,
            "init: KPM bridge not armed (rc=%d). "
            "Run: shctl <KEY> load shpte.kpm && shctl <KEY> control shpte bridge",
            rc);
        return false;
    }

    m_initialised = true;
    __android_log_print(ANDROID_LOG_INFO, TAG,
        "init: KPM bridge live, stealth hook engine ready");
    return true;
}

} // namespace omnibyte::runtime::backends
