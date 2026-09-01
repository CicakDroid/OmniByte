#pragma once
// RootThread — Root service IPC wrapper.
// Source: https://github.com/MMRLApp/RootThread (GPL-3.0)

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace omnibyte::root {

/// Root service request.
struct RootRequest {
    int requestId = 0;
    std::string command;
    std::vector<std::string> arguments;
    int timeoutMs = 30000;
};

/// Root service response.
struct RootResponse {
    int requestId = 0;
    int exitCode = -1;
    std::string stdout;
    std::string stderr;
    bool success = false;
};

/// Root service IPC adapted from RootThread.
/// Provides C++ wrapper for root command execution via socket/pipe.
class RootThread {
public:
    RootThread() = default;
    ~RootThread();

    /// Initialize root service connection.
    bool init();

    /// Execute a root command synchronously.
    RootResponse execSync(const std::string& command,
                          const std::vector<std::string>& args = {},
                          int timeoutMs = 30000);

    /// Execute a root command asynchronously with callback.
    void execAsync(const RootRequest& request,
                   std::function<void(const RootResponse&)> callback);

    /// Check if root service is connected.
    bool isConnected() const;

    /// Close the root service connection.
    void close();

private:
    bool connected_ = false;
    int socketFd_ = -1;
};

}  // namespace omnibyte::root
