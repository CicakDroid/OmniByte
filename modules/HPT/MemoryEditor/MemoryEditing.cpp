// MemoryEditing — memory editing process for HPT.

#include "MemoryEditing.h"

#include <android/log.h>

#define LOG_TAG "HPT.MemoryEditing"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace omnibyte::runtime {

void MemoryEditing::addEditor(std::shared_ptr<runtime::backends::IMemoryEditor> editor) {
    if (editor) {
        editors_.push_back(std::move(editor));
    }
}

bool MemoryEditing::selectEditor(const std::vector<std::string>& priority) {
    for (const auto& name : priority) {
        for (auto& editor : editors_) {
            if (editor->name() == name && editor->isAvailable()) {
                activeEditor_ = editor;
                LOGI("Selected memory editor: %s", name.c_str());
                return true;
            }
        }
    }
    LOGE("No available memory editor found");
    return false;
}

bool MemoryEditing::readMemory(uintptr_t address, void* buffer, size_t size) {
    if (!activeEditor_) {
        LOGE("No active memory editor");
        return false;
    }
    return activeEditor_->readMemory(address, buffer, size);
}

bool MemoryEditing::writeMemory(uintptr_t address, const void* data, size_t size) {
    if (!activeEditor_) {
        LOGE("No active memory editor");
        return false;
    }
    return activeEditor_->writeMemory(address, data, size);
}

bool MemoryEditing::patchMemory(uintptr_t address, const uint8_t* data, size_t size) {
    if (!activeEditor_) {
        LOGE("No active memory editor");
        return false;
    }
    return activeEditor_->patchMemory(address, data, size);
}

bool MemoryEditing::dumpMemory(uintptr_t address, size_t size, const std::string& destPath) {
    if (!activeEditor_) {
        LOGE("No active memory editor");
        return false;
    }
    return activeEditor_->dumpMemory(address, size, destPath);
}

std::string MemoryEditing::activeEditorName() const {
    return activeEditor_ ? activeEditor_->name() : "";
}

bool MemoryEditing::hasActiveEditor() const {
    return activeEditor_ != nullptr;
}

void MemoryEditing::release() {
    activeEditor_.reset();
}

void MemoryEditing::setTargetPid(pid_t pid) {
    targetPid_ = pid;
}

} // namespace omnibyte::runtime
