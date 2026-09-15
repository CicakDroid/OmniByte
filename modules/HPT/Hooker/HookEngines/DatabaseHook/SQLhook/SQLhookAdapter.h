#pragma once
// SQLhookAdapter — SQLite function hooking backend for intercepting database operations.
// Source: https://github.com/CicakDroid/OmniByte (custom implementation)
// Version: 1.0.0
//
// Bridges IHookBackend interface to SQLite native function hooking.
// Intercepts sqlite3_open, sqlite3_exec, sqlite3_prepare_v2, sqlite3_step, sqlite3_close.
// Supports Android 6.0+, ARM/ARM64.
//
// ponytail: Uses ShadowhookAdapter for inline hooking, no external dependency.

#include "../../IHookEngines.h"

#include <string>
#include <unordered_map>
#include <mutex>
#include <functional>
#include <vector>

// SQLite type forward declarations
struct sqlite3;
struct sqlite3_stmt;

namespace omnibyte::runtime::backends {

/// SQLite function hook backend for intercepting database operations.
/// Hooks native SQLite functions in libsqlite.so to monitor/modify SQL queries.
class SQLhookAdapter : public IHookBackend {
public:
    SQLhookAdapter() = default;
    ~SQLhookAdapter() override;

    std::string name() const override { return "SQLhook"; }
    bool isAvailable() const override;

    /// Hook a function at `addr` by rewriting its prologue.
    /// `replacement` is the detour function, `originalOut` receives the trampoline.
    bool hookFunction(uintptr_t addr, void* replacement, void** originalOut) override;

    /// Unhook a previously hooked function, restoring original bytes.
    bool unhook(uintptr_t addr) override;

    /// Raw memory patch (delegates to KittyMemory fallback).
    bool patchMemory(uintptr_t addr, const uint8_t* data, size_t size) override;

    /// Initialize SQLhook adapter. Must be called before any hook operations.
    /// Uses ShadowhookAdapter internally for inline hooking.
    bool init(bool debug = false);

    // --- SQLhook-specific API (beyond IHookBackend) ---

    /// Hook sqlite3_open_v2 to intercept database opens.
    /// callback: called with filename, flags, vfs name before original function executes.
    bool hookSqliteOpen(std::function<void(const char* filename, int flags, const char* vfs)> callback);

    /// Hook sqlite3_exec to intercept SQL execution.
    /// callback: called with SQL statement, error message, callback context.
    bool hookSqliteExec(std::function<void(const char* sql, char* errmsg, void* context)> callback);

    /// Hook sqlite3_prepare_v2 to intercept statement preparation.
    /// callback: called with SQL statement, statement pointer.
    bool hookSqlitePrepare(std::function<void(const char* sql, sqlite3_stmt* stmt)> callback);

    /// Hook sqlite3_step to intercept statement stepping.
    /// callback: called with statement pointer, step result.
    bool hookSqliteStep(std::function<void(sqlite3_stmt* stmt, int result)> callback);

    /// Hook sqlite3_close to intercept database close.
    /// callback: called with database pointer, close result.
    bool hookSqliteClose(std::function<void(sqlite3* db, int result)> callback);

    /// Hook all SQLite functions at once.
    bool hookAll();

    /// Unhook all SQLite functions.
    void unhookAll();

    /// Get SQLhook version string.
    static const char* getVersion();

    /// Check if SQLhook is initialized.
    bool isInitialized() const { return initialized_; }

    /// Get last init error code.
    int getInitError() const { return initError_; }

private:

    bool initialized_ = false;
    int initError_ = -1;

    // Hook registry: address → stub for unhook
    std::unordered_map<uintptr_t, void*> hooks_;
    std::mutex hooksMutex_;

    // Original function pointers
    void* originalSqliteOpenV2_ = nullptr;
    void* originalSqliteExec_ = nullptr;
    void* originalSqlitePrepareV2_ = nullptr;
    void* originalSqliteStep_ = nullptr;
    void* originalSqliteClose_ = nullptr;

    // Callback storage
    std::function<void(const char* filename, int flags, const char* vfs)> onOpen_;
    std::function<void(const char* sql, char* errmsg, void* context)> onExec_;
    std::function<void(const char* sql, sqlite3_stmt* stmt)> onPrepare_;
    std::function<void(sqlite3_stmt* stmt, int result)> onStep_;
    std::function<void(sqlite3* db, int result)> onClose_;
};

} // namespace omnibyte::runtime::backends
