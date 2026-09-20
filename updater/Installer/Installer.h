#pragma once

#include <string>
#include <functional>
#include <vector>

namespace omnibyte::updater {

enum class InstallStatus {
    Success,
    BackupFailed,
    ExtractFailed,
    CopyFailed,
    RollbackFailed,
    PermissionDenied
};

struct InstallResult {
    InstallStatus status = InstallStatus::Success;
    std::string errorMessage;
    std::string backupPath;
};

using InstallLogCallback = std::function<void(const std::string& message)>;

class Installer {
public:
    Installer();
    ~Installer();

    void setLogCallback(InstallLogCallback callback);
    void setBackupDir(const std::string& backupDir);
    void setAutoBackup(bool enabled);

    InstallResult installUpdate(const std::string& archivePath,
                                const std::string& targetDir);
    InstallResult uninstallUpdate(const std::string& installDir);

    bool rollback(const std::string& backupPath, const std::string& targetDir);

    static bool extractArchive(const std::string& archivePath,
                               const std::string& destDir);
    static bool copyDirectory(const std::string& src, const std::string& dest);
    static bool removeDirectory(const std::string& dir);
    static bool createBackup(const std::string& sourceDir, const std::string& backupDir);

private:
    std::string m_backupDir;
    bool m_autoBackup = true;
    InstallLogCallback m_logCallback;

    std::string generateBackupPath() const;
};

} // namespace omnibyte::updater
