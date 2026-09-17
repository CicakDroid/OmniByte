#pragma once

#include <string>
#include <cstdio>
#include <array>
#include <memory>

namespace omnibyte::runtime {

/**
 * Root Filesystem Access via popen("su -c ...")
 * Minimal implementation — no dependencies beyond stdlib.
 *
 * Usage:
 *   auto content = RootFileAccess::readFile("/data/data/com.app/shared_prefs/config.xml");
 *   RootFileAccess::writeFile("/data/local/tmp/hook.config", "key=value");
 */
class RootFileAccess {
public:
    /**
     * Read file content as root.
     * Returns empty string on failure.
     */
    static std::string readFile(const std::string& path) {
        std::string cmd = "su -c cat " + quote(path);
        return exec(cmd);
    }

    /**
     * Write content to file as root.
     * Creates parent directories automatically.
     * Returns true on success.
     */
    static bool writeFile(const std::string& path, const std::string& content) {
        // Escape single quotes in content for shell safety
        std::string escaped = content;
        size_t pos = 0;
        while ((pos = escaped.find("'", pos)) != std::string::npos) {
            escaped.replace(pos, 1, "'\\''");
            pos += 3;
        }

        // Create parent dir + write via tee
        std::string dir = parentDir(path);
        std::string cmd = "su -c \"mkdir -p " + quote(dir) + " && echo '" + escaped + "' > " + quote(path) + "\"";
        std::string result = exec(cmd);
        return result.empty(); // success = no error output
    }

    /**
     * List directory contents as root.
     * Returns newline-separated list, empty on failure.
     */
    static std::string listDir(const std::string& path) {
        std::string cmd = "su -c ls " + quote(path);
        return exec(cmd);
    }

    /**
     * Check if file exists as root.
     */
    static bool fileExists(const std::string& path) {
        std::string cmd = "su -c test -f " + quote(path) + " && echo EXISTS";
        std::string result = exec(cmd);
        return result.find("EXISTS") != std::string::npos;
    }

    /**
     * Check if directory exists as root.
     */
    static bool dirExists(const std::string& path) {
        std::string cmd = "su -c test -d " + quote(path) + " && echo EXISTS";
        std::string result = exec(cmd);
        return result.find("EXISTS") != std::string::npos;
    }

    /**
     * Get file size in bytes as root.
     * Returns -1 on failure.
     */
    static long fileSize(const std::string& path) {
        std::string cmd = "su -c stat -c %s " + quote(path);
        std::string result = exec(cmd);
        try {
            return std::stol(result);
        } catch (...) {
            return -1;
        }
    }

    /**
     * Copy file as root (preserves permissions).
     */
    static bool copyFile(const std::string& src, const std::string& dst) {
        std::string cmd = "su -c cp " + quote(src) + " " + quote(dst);
        return exec(cmd).empty();
    }

    /**
     * Delete file as root.
     */
    static bool deleteFile(const std::string& path) {
        std::string cmd = "su -c rm " + quote(path);
        return exec(cmd).empty();
    }

    /**
     * Check if device has root access.
     */
    static bool isRooted() {
        std::string result = exec("su -c id");
        return result.find("uid=0") != std::string::npos;
    }

public:
    /**
     * Execute shell command via popen, return stdout+stderr.
     */
    static std::string exec(const std::string& cmd) {
        std::string output;
        FILE* pipe = popen(cmd.c_str(), "r");
        if (!pipe) return output;

        std::array<char, 4096> buffer;
        while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr) {
            output += buffer.data();
        }
        pclose(pipe);
        return output;
    }

    /**
     * Quote path for shell safety (wraps in single quotes).
     */
    static std::string quote(const std::string& path) {
        return "'" + path + "'";
    }

    /**
     * Extract parent directory from path.
     */
    static std::string parentDir(const std::string& path) {
        size_t pos = path.find_last_of('/');
        return (pos != std::string::npos) ? path.substr(0, pos) : ".";
    }
};

} // namespace omnibyte::runtime

#ifdef ROOTFILEACCESS_TEST
#include <cassert>
int main() {
    using namespace omnibyte::runtime;
    assert(RootFileAccess::quote("a/b") == "'a/b'");
    assert(RootFileAccess::parentDir("/data/local/tmp/file") == "/data/local/tmp");
    assert(RootFileAccess::parentDir("file.txt") == ".");
    printf("RootFileAccess: self-check passed\n");
    return 0;
}
#endif
