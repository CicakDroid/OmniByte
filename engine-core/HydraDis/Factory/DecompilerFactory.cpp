#include "Factory/DecompilerFactory.h"

// Forward declarations — each backend has a free function in its .cpp file.
namespace omnibyte::hydradis {
    std::unique_ptr<IDecompiler> createRizinNativeDecompiler(
        const std::string& rizinPath
    );
    std::unique_ptr<IDecompiler> createRzGhidraDecompiler();
}

namespace omnibyte::hydradis {

std::unique_ptr<IDecompiler> DecompilerFactory::create(
    DecompilerBackend backend,
    const std::string& rizinPath
) {
    switch (backend) {
        case DecompilerBackend::Auto:
        case DecompilerBackend::RizinNative:
            return createRizinNativeDecompiler(rizinPath);

        case DecompilerBackend::RzGhidra:
            return createRzGhidraDecompiler();

        default:
            return nullptr;
    }
}

std::unique_ptr<IDecompiler> DecompilerFactory::createByCapability(
    DecompilerCapability capability,
    const std::string& rizinPath
) {
    switch (capability) {
        case DecompilerCapability::Light:
            return createRizinNativeDecompiler(rizinPath);

        case DecompilerCapability::Heavy:
            return createRzGhidraDecompiler();

        default:
            return nullptr;
    }
}

} // namespace omnibyte::hydradis
