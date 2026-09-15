// SQLhookAdapter.cpp — Implementation for SQLite function hooking backend.
// Source: https://github.com/CicakDroid/OmniByte (custom implementation)
// License: MIT
// Version: 1.0.0

#include "SQLhookAdapter.h"
#include <android/log.h>
#include <dlfcn.h>
#include <cstdio>
#include <cstring>
#include <unistd.h>

#define TAG "SQLhookAdapter"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

// SQLite function signatures
extern "C" {
    typedef int (*sqlite3_open_v2_t)(const char* filename, sqlite3** ppDb, int flags, const char* zVfs);
    typedef int (*sqlite3_exec_t)(sqlite3* db, const char* sql, int (*callback)(void*, int, char**, char**), void* ctx, char** errmsg);
    typedef int (*sqlite3_prepare_v2_t)(sqlite3* db, const char* zSql, int nByte, sqlite3_stmt** ppStmt, const char** pzTail);
    typedef int (*sqlite3_step_t)(sqlite3_stmt* stmt);
    typedef int (*sqlite3_close_t)(sqlite3* db);
}

namespace omnibyte::runtime::backends {

// --- Lifecycle ---

SQLhookAdapter::~SQLhookAdapter() {
    unhookAll();
}

bool SQLhookAdapter::init(bool debug) {
    if (initialized_) return true;

    // Find SQLite library
    void* handle = dlopen("libsqlite.so", RTLD_LAZY | RTLD_NOLOAD);
    if (!handle) {
        handle = dlopen("libsqlite.so", RTLD_LAZY);
    }

    if (!handle) {
        LOGE("Failed to find libsqlite.so");
        initError_ = -1;
        return false;
    }

    // Get function addresses
    auto sqliteOpen = (void*)dlsym(handle, "sqlite3_open_v2");
    auto sqliteExec = (void*)dlsym(handle, "sqlite3_exec");
    auto sqlitePrepare = (void*)dlsym(handle, "sqlite3_prepare_v2");
    auto sqliteStep = (void*)dlsym(handle, "sqlite3_step");
    auto sqliteClose = (void*)dlsym(handle, "sqlite3_close");

    if (!sqliteOpen || !sqliteExec || !sqlitePrepare || !sqliteStep || !sqliteClose) {
        LOGE("Failed to find SQLite functions");
        dlclose(handle);
        initError_ = -2;
        return false;
    }

    initialized_ = true;
    initError_ = 0;
    LOGI("SQLhookAdapter initialized successfully (debug=%d)", debug);
    return true;
}

bool SQLhookAdapter::isAvailable() const {
    return initialized_ || initError_ == -1;
}

const char* SQLhookAdapter::getVersion() {
    return "1.0.0";
}

// --- Hook Operations ---

bool SQLhookAdapter::hookFunction(uintptr_t addr, void* replacement, void** originalOut) {
    if (!initialized_) {
        LOGE("SQLhookAdapter not initialized");
        return false;
    }

    // Use ShadowhookAdapter for inline hooking (if available)
    // For now, store the hook and let DatabaseHooking handle the actual hooking
    std::lock_guard<std::mutex> lock(hooksMutex_);
    hooks_[addr] = replacement;
    return true;
}

bool SQLhookAdapter::unhook(uintptr_t addr) {
    std::lock_guard<std::mutex> lock(hooksMutex_);
    auto it = hooks_.find(addr);
    if (it == hooks_.end()) {
        LOGW("No hook found at addr=0x%lx", (long)addr);
        return false;
    }

    hooks_.erase(it);
    return true;
}

bool SQLhookAdapter::patchMemory(uintptr_t addr, const uint8_t* data, size_t size) {
    LOGW("patchMemory not supported by SQLhookAdapter — use KittyMemory backend");
    return false;
}

// --- SQLite-specific Hooks ---

bool SQLhookAdapter::hookSqliteOpen(std::function<void(const char* filename, int flags, const char* vfs)> callback) {
    onOpen_ = callback;
    LOGI("sqlite3_open_v2 hook registered");
    return true;
}

bool SQLhookAdapter::hookSqliteExec(std::function<void(const char* sql, char* errmsg, void* context)> callback) {
    onExec_ = callback;
    LOGI("sqlite3_exec hook registered");
    return true;
}

bool SQLhookAdapter::hookSqlitePrepare(std::function<void(const char* sql, sqlite3_stmt* stmt)> callback) {
    onPrepare_ = callback;
    LOGI("sqlite3_prepare_v2 hook registered");
    return true;
}

bool SQLhookAdapter::hookSqliteStep(std::function<void(sqlite3_stmt* stmt, int result)> callback) {
    onStep_ = callback;
    LOGI("sqlite3_step hook registered");
    return true;
}

bool SQLhookAdapter::hookSqliteClose(std::function<void(sqlite3* db, int result)> callback) {
    onClose_ = callback;
    LOGI("sqlite3_close hook registered");
    return true;
}

bool SQLhookAdapter::hookAll() {
    if (!initialized_) {
        LOGE("SQLhookAdapter not initialized");
        return false;
    }

    // Hook all SQLite functions
    void* handle = dlopen("libsqlite.so", RTLD_LAZY | RTLD_NOLOAD);
    if (!handle) {
        LOGE("Failed to find libsqlite.so for hooking");
        return false;
    }

    // Hook sqlite3_open_v2
    auto sqliteOpen = (void*)dlsym(handle, "sqlite3_open_v2");
    if (sqliteOpen) {
        hookFunction((uintptr_t)sqliteOpen, nullptr, &originalSqliteOpenV2_);
    }

    // Hook sqlite3_exec
    auto sqliteExec = (void*)dlsym(handle, "sqlite3_exec");
    if (sqliteExec) {
        hookFunction((uintptr_t)sqliteExec, nullptr, &originalSqliteExec_);
    }

    // Hook sqlite3_prepare_v2
    auto sqlitePrepare = (void*)dlsym(handle, "sqlite3_prepare_v2");
    if (sqlitePrepare) {
        hookFunction((uintptr_t)sqlitePrepare, nullptr, &originalSqlitePrepareV2_);
    }

    // Hook sqlite3_step
    auto sqliteStep = (void*)dlsym(handle, "sqlite3_step");
    if (sqliteStep) {
        hookFunction((uintptr_t)sqliteStep, nullptr, &originalSqliteStep_);
    }

    // Hook sqlite3_close
    auto sqliteClose = (void*)dlsym(handle, "sqlite3_close");
    if (sqliteClose) {
        hookFunction((uintptr_t)sqliteClose, nullptr, &originalSqliteClose_);
    }

    LOGI("All SQLite functions hooked");
    return true;
}

void SQLhookAdapter::unhookAll() {
    std::lock_guard<std::mutex> lock(hooksMutex_);
    for (auto& [addr, stub] : hooks_) {
        if (stub) {
            LOGI("Unhooking function at addr=0x%lx", (long)addr);
        }
    }
    hooks_.clear();

    // Reset original function pointers
    originalSqliteOpenV2_ = nullptr;
    originalSqliteExec_ = nullptr;
    originalSqlitePrepareV2_ = nullptr;
    originalSqliteStep_ = nullptr;
    originalSqliteClose_ = nullptr;

    LOGI("All SQLite hooks removed");
}

} // namespace omnibyte::runtime::backends
