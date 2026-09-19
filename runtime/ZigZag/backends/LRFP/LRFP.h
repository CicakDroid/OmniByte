#pragma once
// LRFP — Low-level Rooting Frameworks Plugins bypass.
// Source: https://github.com/LRFP-Team/Bypasser (MIT)
//
// Systematically bypasses environment detection:
//   - Root detection (su, Magisk, KernelSU, APatch)
//   - Frida detection (frida-server, frida-gadget, named pipes)
//   - Xposed detection (XposedBridge class, libxposed)
//   - Magisk detection (MagiskManager, props, mounts)
//   - Debugger detection (tracerpid, TracerPid)
//   - Emulator detection (build props, files)

#include "IStealthBackend.h"

#include <string>
#include <vector>

namespace omnibyte::runtime::backends {

class LRFP : public IStealthBackend {
public:
    LRFP() = default;
    ~LRFP() = default;

    std::string name() const override { return "LRFP"; }
    bool isAvailable() const override;
    bool hide(pid_t pid) override;
    bool unhide(pid_t pid) override;
    bool bypassPtraceScope() override;
    bool bypassSelinuxDenial() override;

    // --- LRFP-specific API ---

    /// Bypass root detection (Magisk, KernelSU, APatch, su binary).
    bool bypassRootDetection();

    /// Bypass Frida detection (frida-server, frida-gadget, named pipes).
    bool bypassFridaDetection();

    /// Bypass Xposed detection (XposedBridge, libxposed_art, de.robv.android.xposed).
    bool bypassXposedDetection();

    /// Bypass Magisk detection (MagiskManager, hide props, mounts).
    bool bypassMagiskDetection();

    /// Bypass debugger detection (TracerPid, ptrace).
    bool bypassDebuggerDetection();

    /// Bypass all detection methods at once.
    bool bypassAll();

private:
    bool active_ = false;

    /// Check if a file exists.
    bool fileExists(const char* path) const;

    /// Check if a process is running by name.
    bool isProcessRunning(const char* name) const;

    /// Hide a file by renaming it with null bytes.
    bool hideFile(const char* path);

    /// Restore a previously hidden file.
    bool restoreFile(const char* path);
};

} // namespace omnibyte::runtime::backends
