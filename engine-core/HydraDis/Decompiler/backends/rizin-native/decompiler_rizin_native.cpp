#include "Decompiler/IDecompiler.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <memory>
#include <stdexcept>

// Forward declare rizin types to avoid pulling in all of rizin's headers
// when only the C API is needed via popen.
struct RzCore;

namespace omnibyte::hydradis {

class RizinNativeDecompiler final : public IDecompiler {
public:
    explicit RizinNativeDecompiler(const std::string& rizinPath = "rizin")
        : rizinPath_(rizinPath) {}

    std::string name() const override {
        return "rizin-native";
    }

    DecompilerCapability capability() const override {
        return DecompilerCapability::Light;
    }

    DecompiledFunction decompile(
        const uint8_t* code,
        size_t codeSize,
        uint64_t baseAddr
    ) const override {
        DecompiledFunction result;

        if (!code || codeSize == 0) {
            result.errorMessage = "Empty code buffer";
            return result;
        }

        // Write raw bytes to a temp file so rizin can load them
        // Using malloc:// via rizin's stdin pipe would be cleaner but
        // requires rizin C API access. Temp file is reliable fallback.
        char tmpPath[] = "/tmp/rizin_XXXXXX";
        int tmpFd = mkstemp(tmpPath);
        if (tmpFd < 0) {
            result.errorMessage = "Failed to create temp file";
            return result;
        }

        ssize_t written = write(tmpFd, code, codeSize);
        close(tmpFd);

        if (written != static_cast<ssize_t>(codeSize)) {
            unlink(tmpPath);
            result.errorMessage = "Failed to write code to temp file";
            return result;
        }

        // Build rizin command: open file, seek to baseAddr, disassemble codeSize/4 instructions
        // "-q" = quiet, "-e scr.color=0" = no color codes in output
        std::string cmd = rizinPath_ +
            " -q -e scr.color=0 -c \"s " + std::to_string(baseAddr) +
            " ; pd " + std::to_string(codeSize / 4) +
            "\" " + std::string(tmpPath) + " 2>/dev/null";

        FILE* pipe = popen(cmd.c_str(), "r");
        if (!pipe) {
            unlink(tmpPath);
            result.errorMessage = "Failed to execute rizin";
            return result;
        }

        std::string output;
        char buffer[4096];
        while (fgets(buffer, sizeof(buffer), pipe)) {
            output += buffer;
        }

        int status = pclose(pipe);
        unlink(tmpPath);

        if (status != 0) {
            result.errorMessage = "rizin exited with status " + std::to_string(status);
            return result;
        }

        result.success = true;
        result.address = baseAddr;
        result.pseudocode = output;
        return result;
    }

private:
    std::string rizinPath_;
};

std::unique_ptr<IDecompiler> createRizinNativeDecompiler(
    const std::string& rizinPath
) {
    return std::make_unique<RizinNativeDecompiler>(rizinPath);
}

} // namespace omnibyte::hydradis
