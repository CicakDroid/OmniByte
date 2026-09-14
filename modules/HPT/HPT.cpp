// HPT — Hooking Platform Toolkit orchestrator.

#include "HPT.h"
#include "modules/Dumper/Dumper.h"

namespace omnibyte::runtime {

omnibyte::dumper::DumpResult HPT::selectBackend(
    const std::vector<std::string>& hookBackendPriority) {
    if (hooking_.hasActiveBackend()) return omnibyte::dumper::DumpResult::Success;
    if (hooking_.selectBackend(hookBackendPriority)) {
        return omnibyte::dumper::DumpResult::Success;
    }
    return omnibyte::dumper::DumpResult::HookFailed;
}

bool HPT::hookFunction(uintptr_t addr, void* replacement, void** originalOut) {
    if (!hooking_.hasActiveBackend()) return false;
    auto entry = hooking_.installHook(addr, replacement);
    if (entry.installed && originalOut) {
        *originalOut = entry.original;
    }
    return entry.installed;
}

bool HPT::unhook(uintptr_t addr) {
    return hooking_.uninstallHook(addr);
}

bool HPT::patchMemory(uintptr_t addr, const uint8_t* data, size_t size) {
    return hooking_.patchMemory(addr, data, size);
}

std::string HPT::activeBackendName() const {
    return hooking_.activeBackendName();
}

void HPT::registerBackend(std::shared_ptr<IHookBackend> backend) {
    hooking_.addBackend(std::move(backend));
}

bool HPT::selectEditor(const std::vector<std::string>& editorPriority) {
    return memoryEditing_.selectEditor(editorPriority);
}

bool HPT::readMemory(uintptr_t address, void* buffer, size_t size) {
    return memoryEditing_.readMemory(address, buffer, size);
}

bool HPT::writeMemory(uintptr_t address, const void* data, size_t size) {
    return memoryEditing_.writeMemory(address, data, size);
}

bool HPT::patchMemoryEditor(uintptr_t address, const uint8_t* data, size_t size) {
    return memoryEditing_.patchMemory(address, data, size);
}

bool HPT::dumpMemory(uintptr_t address, size_t size, const std::string& destPath) {
    return memoryEditing_.dumpMemory(address, size, destPath);
}

void HPT::registerEditor(std::shared_ptr<IMemoryEditor> editor) {
    memoryEditing_.addEditor(std::move(editor));
}

std::string HPT::activeEditorName() const {
    return memoryEditing_.activeEditorName();
}

void HPT::release() {
    hooking_.release();
    memoryEditing_.release();
}

} // namespace omnibyte::runtime
