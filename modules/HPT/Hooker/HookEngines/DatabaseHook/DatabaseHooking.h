#pragma once
// DatabaseHooking — Abstraction layer for database hooking process.
// Source: https://github.com/CicakDroid/OmniByte (custom implementation)
// Version: 1.0.0
//
// Provides high-level database hooking operations using SQLhookAdapter.
// Supports SQLite database interception, query logging, and modification.
//
// ponytail: Minimal abstraction, direct adapter usage.

#include "SQLhook/SQLhookAdapter.h"

#include <string>
#include <functional>
#include <vector>
#include <memory>

namespace omnibyte::runtime::backends {

/// Database hooking process abstraction layer.
/// Provides high-level API for intercepting and modifying database operations.
class DatabaseHooking {
public:
    DatabaseHooking() = default;
    ~DatabaseHooking() = default;

    /// Initialize database hooking with specified adapter.
    bool init(std::shared_ptr<SQLhookAdapter> adapter);

    /// Hook all SQLite functions for monitoring.
    bool hookAll();

    /// Unhook all SQLite functions.
    void unhookAll();

    /// Hook specific SQLite function: sqlite3_open_v2.
    bool hookSqliteOpen(std::function<void(const char* filename, int flags, const char* vfs)> callback);

    /// Hook specific SQLite function: sqlite3_exec.
    bool hookSqliteExec(std::function<void(const char* sql, char* errmsg, void* context)> callback);

    /// Hook specific SQLite function: sqlite3_prepare_v2.
    bool hookSqlitePrepare(std::function<void(const char* sql, sqlite3_stmt* stmt)> callback);

    /// Hook specific SQLite function: sqlite3_step.
    bool hookSqliteStep(std::function<void(sqlite3_stmt* stmt, int result)> callback);

    /// Hook specific SQLite function: sqlite3_close.
    bool hookSqliteClose(std::function<void(sqlite3* db, int result)> callback);

    /// Get list of all hooked database files.
    std::vector<std::string> getHookedDatabases() const;

    /// Get list of all intercepted SQL queries.
    std::vector<std::string> getInterceptedQueries() const;

    /// Clear intercepted queries log.
    void clearInterceptedQueries();

    /// Check if database hooking is initialized.
    bool isInitialized() const { return initialized_; }

    /// Get adapter version.
    const char* getAdapterVersion() const;

private:
    bool initialized_ = false;
    std::shared_ptr<SQLhookAdapter> adapter_;

    // Hooked databases tracking
    std::vector<std::string> hookedDatabases_;
    std::vector<std::string> interceptedQueries_;
};

} // namespace omnibyte::runtime::backends
