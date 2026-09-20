#include "Installer.h"
#include <fstream>
#include <filesystem>
#include <cstdlib>
#include <chrono>
#include <sstream>
#include <algorithm>

namespace fs = std::filesystem;

namespace omnibyte::updater {

Installer::Installer() = default;
Installer::~Installer() = default;

void Installer::setLogCallback(InstallLogCallback cb) { m_logCallback = std::move(cb); }
void Installer::setBackupDir(const std::string& dir) { m_backupDir = dir; }
void Installer::setAutoBackup(bool enabled) { m_autoBackup = enabled; }

std::string Installer::generateBackupPath() const {
    auto now = std::chrono::system_clock::now();
    auto epoch = now.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(epoch).count();

    std::string base = m_backupDir.empty() ? "." : m_backupDir;
    return base + "/backup_" + std::to_string(millis);
}

bool Installer::createBackup(const std::string& sourceDir, const std::string& backupDir) {
    std::error_code ec;

    if (!fs::exists(sourceDir)) return false;

    fs::create_directories(backupDir, ec);
    if (ec) return false;

    fs::copy(sourceDir, backupDir, fs::copy_options::recursive | fs::copy_options::overwrite_existing, ec);
    return !ec;
}

bool Installer::extractArchive(const std::string& archivePath, const std::string& destDir) {
    std::error_code ec;
    fs::create_directories(destDir, ec);
    if (ec) return false;

    int ret = system(("unzip -o -q \"" + archivePath + "\" -d \"" + destDir + "\"").c_str());
    return ret == 0;
}

bool Installer::copyDirectory(const std::string& src, const std::string& dest) {
    std::error_code ec;
    fs::create_directories(dest, ec);
    if (ec) return false;

    fs::copy(src, dest, fs::copy_options::recursive | fs::copy_options::overwrite_existing, ec);
    return !ec;
}

bool Installer::removeDirectory(const std::string& dir) {
    std::error_code ec;
    fs::remove_all(dir, ec);
    return !ec;
}

bool Installer::rollback(const std::string& backupPath, const std::string& targetDir) {
    if (m_logCallback) m_logCallback("Rolling back from: " + backupPath);

    if (!fs::exists(backupPath)) {
        if (m_logCallback) m_logCallback("Backup not found: " + backupPath);
        return false;
    }

    removeDirectory(targetDir);
    return copyDirectory(backupPath, targetDir);
}

InstallResult Installer::installUpdate(const std::string& archivePath,
                                       const std::string& targetDir) {
    if (!fs::exists(archivePath)) {
        return {InstallStatus::ExtractFailed, "Archive not found: " + archivePath};
    }

    std::string backupPath;
    if (m_autoBackup && fs::exists(targetDir)) {
        backupPath = generateBackupPath();
        if (m_logCallback) m_logCallback("Creating backup: " + backupPath);

        if (!createBackup(targetDir, backupPath)) {
            return {InstallStatus::BackupFailed, "Failed to create backup", backupPath};
        }
    }

    std::string extractDir = targetDir + "_extract_tmp";
    if (m_logCallback) m_logCallback("Extracting archive...");

    if (!extractArchive(archivePath, extractDir)) {
        if (!backupPath.empty()) rollback(backupPath, targetDir);
        removeDirectory(extractDir);
        return {InstallStatus::ExtractFailed, "Failed to extract archive"};
    }

    if (m_logCallback) m_logCallback("Copying files to target...");

    if (!copyDirectory(extractDir, targetDir)) {
        if (!backupPath.empty()) rollback(backupPath, targetDir);
        removeDirectory(extractDir);
        return {InstallStatus::CopyFailed, "Failed to copy files to target"};
    }

    removeDirectory(extractDir);

    if (m_logCallback) m_logCallback("Update installed successfully");
    return {InstallStatus::Success, "", backupPath};
}

InstallResult Installer::uninstallUpdate(const std::string& installDir) {
    if (!fs::exists(installDir)) {
        return {InstallStatus::Success, "Nothing to uninstall"};
    }

    if (m_logCallback) m_logCallback("Removing: " + installDir);

    if (!removeDirectory(installDir)) {
        return {InstallStatus::PermissionDenied, "Failed to remove directory"};
    }

    return {InstallStatus::Success};
}

} // namespace omnibyte::updater
