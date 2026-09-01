#include "Decompiler/IDecompiler.h"
#include <cstdio>
#include <memory>
#include <stdexcept>

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

        std::string cmd = rizinPath_ + " -q -e scr.color=0 -c \"pd " +
                          std::to_string(codeSize / 4) + " @ " +
                          std::to_string(baseAddr) + "\" 2>/dev/null";

        FILE* pipe = popen(cmd.c_str(), "r");
        if (!pipe) {
            result.errorMessage = "Failed to execute rizin";
            return result;
        }

        std::string output;
        char buffer[4096];
        while (fgets(buffer, sizeof(buffer), pipe)) {
            output += buffer;
        }

        int status = pclose(pipe);
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
