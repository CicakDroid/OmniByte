#pragma once
// Sui — Modern SuperUser management wrapper.
// Source: https://github.com/XiaoTong6666/Sui (GPL-3.0)

#include <cstdint>
#include <string>
#include <vector>

namespace omnibyte::bypass {

/// SUID grant request result.
struct SuiGrantResult {
    bool granted = false;
    std::string packageName;
    int uid = 0;
};

/// SuperUser access management adapted from Sui.
/// Provides programmatic SU access control.
class SuiManager {
public:
    SuiManager() = default;
    ~SuiManager() = default;

    /// Initialize the Sui manager (connect to su daemon).
    bool init();

    /// Check if Sui is installed and accessible.
    bool isAvailable() const;

    /// Execute a command as root.
    /// @return stdout output, or empty string on failure.
    std::string execRoot(const std::string& command);

    /// Grant root access to a package.
    bool grantRoot(const std::string& packageName, int uid);

    /// Revoke root access from a package.
    bool revokeRoot(const std::string& packageName);

    /// Get list of packages with root access.
    std::vector<std::string> getGrantedPackages() const;

    /// Check if a specific package has root access.
    bool hasRoot(const std::string& packageName, int uid) const;

private:
    bool connected_ = false;
};

}  // namespace omnibyte::bypass
