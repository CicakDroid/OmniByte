#pragma once

#include "RootFileAccess.h"
#include <string>
#include <vector>

namespace omnibyte::runtime {

/**
 * Dynamic Android Permission Switcher via root.
 * Grants/revokes permissions at runtime using `pm grant/revoke`.
 *
 * Usage:
 *   DynamicPermissionSwitcher::grant("com.target.app", "android.permission.READ_CONTACTS");
 *   DynamicPermissionSwitcher::revoke("com.target.app", "android.permission.CAMERA");
 *   auto perms = DynamicPermissionSwitcher::listGranted("com.target.app");
 */
class DynamicPermissionSwitcher {
public:
    struct PermResult {
        bool ok = false;
        std::string permission;
        std::string errorMessage;
    };

    /**
     * Grant a single permission to target package.
     */
    static PermResult grant(const std::string& packageName,
                           const std::string& permission) {
        PermResult result;
        result.permission = permission;

        std::string cmd = "su -c pm grant " + quote(packageName) + " " + quote(permission);
        std::string output = RootFileAccess::exec(cmd);

        if (output.find("Exception") != std::string::npos ||
            output.find("not found") != std::string::npos) {
            result.errorMessage = output;
            return result;
        }

        result.ok = true;
        return result;
    }

    /**
     * Revoke a single permission from target package.
     */
    static PermResult revoke(const std::string& packageName,
                            const std::string& permission) {
        PermResult result;
        result.permission = permission;

        std::string cmd = "su -c pm revoke " + quote(packageName) + " " + quote(permission);
        std::string output = RootFileAccess::exec(cmd);

        if (output.find("Exception") != std::string::npos ||
            output.find("not found") != std::string::npos) {
            result.errorMessage = output;
            return result;
        }

        result.ok = true;
        return result;
    }

    /**
     * Grant multiple permissions at once.
     * Returns number of successful grants.
     */
    static int grantMultiple(const std::string& packageName,
                            const std::vector<std::string>& permissions) {
        int success = 0;
        for (const auto& perm : permissions) {
            if (grant(packageName, perm).ok) {
                success++;
            }
        }
        return success;
    }

    /**
     * Revoke multiple permissions at once.
     * Returns number of successful revocations.
     */
    static int revokeMultiple(const std::string& packageName,
                             const std::vector<std::string>& permissions) {
        int success = 0;
        for (const auto& perm : permissions) {
            if (revoke(packageName, perm).ok) {
                success++;
            }
        }
        return success;
    }

    /**
     * List all granted permissions for a package.
     * Returns newline-separated permission names.
     */
    static std::string listGranted(const std::string& packageName) {
        std::string cmd = "su -c pm dump " + quote(packageName) + " | grep permission";
        std::string output = RootFileAccess::exec(cmd);

        std::string result;
        std::istringstream stream(output);
        std::string line;
        while (std::getline(stream, line)) {
            if (line.find("granted=true") != std::string::npos) {
                size_t start = line.find("android.permission.");
                if (start == std::string::npos) start = line.find("com.");
                if (start != std::string::npos) {
                    size_t end = line.find(' ', start);
                    std::string perm = (end != std::string::npos)
                        ? line.substr(start, end - start)
                        : line.substr(start);
                    result += perm + "\n";
                }
            }
        }
        return result;
    }

    /**
     * Check if a specific permission is granted.
     */
    static bool isGranted(const std::string& packageName,
                         const std::string& permission) {
        std::string cmd = "su -c pm dump " + quote(packageName) +
                         " | grep " + quote(permission);
        std::string output = RootFileAccess::exec(cmd);
        return output.find("granted=true") != std::string::npos;
    }

    /**
     * List all installed packages.
     * Returns newline-separated package names.
     */
    static std::string listPackages() {
        return RootFileAccess::exec("su -c pm list packages");
    }

    /**
     * Grant all dangerous permissions (runtime permissions) to a package.
     * Uses dumpsys to find which permissions are NOT yet granted, then grants them.
     */
    static int grantAllDangerous(const std::string& packageName) {
        static const std::vector<std::string> dangerous = {
            "android.permission.READ_CONTACTS",
            "android.permission.WRITE_CONTACTS",
            "android.permission.GET_ACCOUNTS",
            "android.permission.READ_CALENDAR",
            "android.permission.WRITE_CALENDAR",
            "android.permission.CAMERA",
            "android.permission.READ_EXTERNAL_STORAGE",
            "android.permission.WRITE_EXTERNAL_STORAGE",
            "android.permission.READ_MEDIA_IMAGES",
            "android.permission.READ_MEDIA_VIDEO",
            "android.permission.READ_MEDIA_AUDIO",
            "android.permission.RECORD_AUDIO",
            "android.permission.ACCESS_FINE_LOCATION",
            "android.permission.ACCESS_COARSE_LOCATION",
            "android.permission.READ_PHONE_STATE",
            "android.permission.CALL_PHONE",
            "android.permission.READ_CALL_LOG",
            "android.permission.WRITE_CALL_LOG",
            "android.permission.SEND_SMS",
            "android.permission.RECEIVE_SMS",
            "android.permission.READ_SMS",
            "android.permission.BODY_SENSORS",
            "android.permission.ACTIVITY_RECOGNITION",
            "android.permission.POST_NOTIFICATIONS",
            "android.permission.READ_MEDIA_VISUAL_USER_SELECTED",
        };

        int granted = 0;
        for (const auto& perm : dangerous) {
            if (!isGranted(packageName, perm)) {
                if (grant(packageName, perm).ok) {
                    granted++;
                }
            }
        }
        return granted;
    }

private:
    static std::string quote(const std::string& s) {
        return "'" + s + "'";
    }
};

} // namespace omnibyte::runtime

#ifdef DYNAMICPERMISSIONSWITCHER_TEST
#include <cassert>
int main() {
    printf("DynamicPermissionSwitcher: compile-check passed\n");
    return 0;
}
#endif
