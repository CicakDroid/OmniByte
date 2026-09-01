// stealth-poc — Kernel traceless hooking via KPM.
// Source: https://github.com/1013503897/stealth-poc
// Commit: main branch, 2026-09-01
// Intercepts execution without modifying target memory.
// Survives CRC/maps-scan detection.

#include "TracelessHook.h"

#include <cstring>
#include <fstream>
#include <linux/kpm.h>
#include <sys/ioctl.h>
#include <unistd.h>

namespace omnibyte::hook {

static constexpr const char* KPM_DEVICE = "/dev/kpm";
static constexpr uint32_t IOCTL_INSTALL_HOOK = 0x1001;
static constexpr uint32_t IOCTL_REMOVE_HOOK = 0x1002;

class KpmTracelessHook : public TracelessHookEngine {
public:
    bool init() override {
        kpmFd_ = open(KPM_DEVICE, O_RDWR);
        return kpmFd_ >= 0;
    }

    bool installHook(uintptr_t target, uintptr_t replacement) override {
        if (kpmFd_ < 0) return false;

        struct kpm_hook_request {
            uintptr_t target;
            uintptr_t replacement;
        } req{target, replacement};

        return ioctl(kpmFd_, IOCTL_INSTALL_HOOK, &req) == 0;
    }

    bool removeHook(uintptr_t target) override {
        if (kpmFd_ < 0) return false;
        return ioctl(kpmFd_, IOCTL_REMOVE_HOOK, &target) == 0;
    }

    bool isHookActive(uintptr_t target) const override {
        // In production: query KPM for active hooks
        (void)target;
        return false;
    }

    size_t getActiveHookCount() const override {
        return 0;
    }

    ~KpmTracelessHook() override {
        if (kpmFd_ >= 0) close(kpmFd_);
    }

private:
    int kpmFd_ = -1;
};

// Factory function — returns nullptr if KPM not available
TracelessHookEngine* createKpmHook() {
    auto* hook = new KpmTracelessHook();
    if (!hook->init()) {
        delete hook;
        return nullptr;
    }
    return hook;
}

}  // namespace omnibyte::hook
