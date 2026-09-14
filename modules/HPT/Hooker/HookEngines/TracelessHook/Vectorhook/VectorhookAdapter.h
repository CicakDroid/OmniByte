/**
 * @file VectorhookAdapter.h
 * @brief Stealth KPM inline-hook adapter — wraps 1013503897/stealth-core (MIT)
 *        kpmhook API onto IHookBackend.
 *
 *  Engine : shpte whole-page DBI clone via no-superkey sysinfo bridge (arm64 only)
 *  Commit : v0.1.0 (2026-09-05)
 *  License: MIT (stealth-core); Vector itself is GPL-3.0 but this adapter wraps only the MIT layer
 *  Source : https://github.com/1013503897/stealth-core
 *  Refs   : demo shows kpm_hook_force_enable() → kpm_hook_init() →
 *           kpm_inline_hooker(target, hooker) → backup → kpm_inline_unhooker() → kpm_hook_shutdown()
 */

#pragma once

#include "../../IHookEngines.h"

// Forward-declare kpmhook C API (no headers shipped — symbol resolved at load time)
extern "C" {
void kpm_hook_force_enable(void);
int  kpm_hook_init(void);
void *kpm_inline_hooker(void *target, void *hooker);
int  kpm_inline_unhooker(void *func);
void kpm_hook_shutdown(void);
}

namespace omnibyte::runtime::backends {

class VectorhookAdapter : public IHookBackend {
public:
    VectorhookAdapter();
    ~VectorhookAdapter() override;

    // IHookBackend
    std::string name() const override;
    bool        isAvailable() const override;
    bool        hookFunction(uintptr_t address, void* hookFn, void** origFn) override;
    bool        unhook(uintptr_t address) override;
    bool        patchMemory(uintptr_t address, const uint8_t* data, size_t size) override;

private:
    bool m_initialised;
    bool m_available;

    bool init();
};

} // namespace omnibyte::runtime::backends
