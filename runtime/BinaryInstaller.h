#pragma once

#include "RootFileAccess.h"
#include <string>
#include <vector>

namespace omnibyte::runtime {

/**
 * Install/uninstall native libraries (.so) into target app via root.
 * Uses RootFileAccess for all filesystem operations.
 *
 * Usage:
 *   BinaryInstaller::install("com.target.app", "/data/local/tmp/libhook.so", "arm64");
 *   auto libs = BinaryInstaller::listInstalled("com.target.app");
 */
class BinaryInstaller {
public:
    /**
     * Install .so file into target app's lib directory.
     * Creates directory if needed, sets permissions (755).
     * Returns true on success.
     */
    static bool install(const std::string& packageName,
                       const std::string& soPath,
                       const std::string& arch = "arm64") {
        std::string destDir = getLibDir(packageName, arch);
        std::string destPath = destDir + "/" + getFileName(soPath);

        if (!RootFileAccess::dirExists(destDir)) {
            RootFileAccess::exec("su -c mkdir -p " + quote(destDir));
        }

        if (!RootFileAccess::copyFile(soPath, destPath)) {
            return false;
        }

        RootFileAccess::exec("su -c chmod 755 " + quote(destPath));
        RootFileAccess::exec("su -c chown $(stat -c %u:%g " + quote(destDir) + ") " + quote(destPath));

        return RootFileAccess::fileExists(destPath);
    }

    /**
     * Install multiple .so files at once.
     * Returns number of successful installations.
     */
    static int installMultiple(const std::string& packageName,
                              const std::vector<std::pair<std::string, std::string>>& files,
                              const std::string& arch = "arm64") {
        int success = 0;
        for (const auto& [src, name] : files) {
            if (install(packageName, src, arch)) {
                success++;
            }
        }
        return success;
    }

    /**
     * Uninstall (delete) .so file from target app.
     */
    static bool uninstall(const std::string& packageName,
                         const std::string& libName,
                         const std::string& arch = "arm64") {
        std::string path = getLibDir(packageName, arch) + "/" + libName;
        return RootFileAccess::deleteFile(path);
    }

    /**
     * List installed .so files in target app's lib directory.
     * Returns newline-separated list of filenames.
     */
    static std::string listInstalled(const std::string& packageName,
                                    const std::string& arch = "arm64") {
        std::string dir = getLibDir(packageName, arch);
        if (!RootFileAccess::dirExists(dir)) {
            return "";
        }
        return RootFileAccess::listDir(dir);
    }

    /**
     * Check if a specific .so is installed in target app.
     */
    static bool isInstalled(const std::string& packageName,
                           const std::string& libName,
                           const std::string& arch = "arm64") {
        std::string path = getLibDir(packageName, arch) + "/" + libName;
        return RootFileAccess::fileExists(path);
    }

    /**
     * Get the lib directory path for a target app.
     * Supports multiple Android versions.
     */
    static std::string getLibDir(const std::string& packageName,
                                const std::string& arch = "arm64") {
        std::string base = "/data/app/" + packageName;

        std::string check = RootFileAccess::exec(
            "su -c test -d " + quote(base) + " && echo EXISTS"
        );
        if (check.find("EXISTS") != std::string::npos) {
            return base + "/lib/" + arch;
        }

        return "/data/data/" + packageName + "/files/lib/" + arch;
    }

    /**
     * Get package name from installed app path.
     * Useful when iterating /data/app/ directories.
     */
    static std::string packageNameFromPath(const std::string& appPath) {
        size_t lastSlash = appPath.find_last_of('/');
        std::string dirName = (lastSlash != std::string::npos)
            ? appPath.substr(lastSlash + 1)
            : appPath;

        size_t dashPos = dirName.find('-');
        if (dashPos != std::string::npos) {
            return dirName.substr(0, dashPos);
        }
        return dirName;
    }

private:
    static std::string getFileName(const std::string& path) {
        size_t pos = path.find_last_of('/');
        return (pos != std::string::npos) ? path.substr(pos + 1) : path;
    }

    static std::string quote(const std::string& s) {
        return "'" + s + "'";
    }
};

} // namespace omnibyte::runtime

#ifdef BINARYINSTALLER_TEST
#include <cassert>
int main() {
    using namespace omnibyte::runtime;
    assert(BinaryInstaller::packageNameFromPath("/data/app/com.example-123") == "com.example");
    assert(BinaryInstaller::packageNameFromPath("com.example") == "com.example");
    assert(BinaryInstaller::getFileName("/data/local/tmp/libhook.so") == "libhook.so");
    assert(BinaryInstaller::getFileName("libhook.so") == "libhook.so");
    printf("BinaryInstaller: self-check passed\n");
    return 0;
}
#endif
