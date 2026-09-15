// DatabaseHooking.cpp — Implementation for database hooking process abstraction.
// Source: https://github.com/CicakDroid/OmniByte (custom implementation)
// License: MIT
// Version: 1.0.0

#include "DatabaseHooking.h"
#include <android/log.h>

#define TAG "DatabaseHooking"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

namespace omnibyte::runtime::backends {

bool DatabaseHooking::init(std::shared_ptr<SQLhookAdapter> adapter) {
    if (!adapter) {
        LOGE("Null adapter provided");
        return false;
    }

    adapter_ = adapter;
    initialized_ = true;
    LOGI("DatabaseHooking initialized with adapter: %s", adapter_->name().c_str());
    return true;
}

bool DatabaseHooking::hookAll() {
    if (!initialized_ || !adapter_) {
        LOGE("DatabaseHooking not initialized");
        return false;
    }

    // Hook all SQLite functions
    adapter_->hookAll();
    LOGI("All SQLite functions hooked");
    return true;
}

void DatabaseHooking::unhookAll() {
    if (!initialized_ || !adapter_) {
        LOGE("DatabaseHooking not initialized");
        return;
    }

    adapter_->unhookAll();
    hookedDatabases_.clear();
    interceptedQueries_.clear();
    LOGI("All SQLite hooks removed");
}

bool DatabaseHooking::hookSqliteOpen(std::function<void(const char* filename, int flags, const char* vfs)> callback) {
    if (!initialized_ || !adapter_) {
        LOGE("DatabaseHooking not initialized");
        return false;
    }

    // Wrap callback to track hooked databases
    auto wrappedCallback = [this, callback](const char* filename, int flags, const char* vfs) {
        if (filename) {
            hookedDatabases_.push_back(filename);
            LOGI("Database opened: %s", filename);
        }
        callback(filename, flags, vfs);
    };

    return adapter_->hookSqliteOpen(wrappedCallback);
}

bool DatabaseHooking::hookSqliteExec(std::function<void(const char* sql, char* errmsg, void* context)> callback) {
    if (!initialized_ || !adapter_) {
        LOGE("DatabaseHooking not initialized");
        return false;
    }

    // Wrap callback to track intercepted queries
    auto wrappedCallback = [this, callback](const char* sql, char* errmsg, void* context) {
        if (sql) {
            interceptedQueries_.push_back(sql);
            LOGI("SQL executed: %s", sql);
        }
        callback(sql, errmsg, context);
    };

    return adapter_->hookSqliteExec(wrappedCallback);
}

bool DatabaseHooking::hookSqlitePrepare(std::function<void(const char* sql, sqlite3_stmt* stmt)> callback) {
    if (!initialized_ || !adapter_) {
        LOGE("DatabaseHooking not initialized");
        return false;
    }

    return adapter_->hookSqlitePrepare(callback);
}

bool DatabaseHooking::hookSqliteStep(std::function<void(sqlite3_stmt* stmt, int result)> callback) {
    if (!initialized_ || !adapter_) {
        LOGE("DatabaseHooking not initialized");
        return false;
    }

    return adapter_->hookSqliteStep(callback);
}

bool DatabaseHooking::hookSqliteClose(std::function<void(sqlite3* db, int result)> callback) {
    if (!initialized_ || !adapter_) {
        LOGE("DatabaseHooking not initialized");
        return false;
    }

    return adapter_->hookSqliteClose(callback);
}

std::vector<std::string> DatabaseHooking::getHookedDatabases() const {
    return hookedDatabases_;
}

std::vector<std::string> DatabaseHooking::getInterceptedQueries() const {
    return interceptedQueries_;
}

void DatabaseHooking::clearInterceptedQueries() {
    interceptedQueries_.clear();
    LOGI("Intercepted queries cleared");
}

const char* DatabaseHooking::getAdapterVersion() const {
    if (!initialized_ || !adapter_) {
        return "not initialized";
    }
    return SQLhookAdapter::getVersion();
}

} // namespace omnibyte::runtime::backends
