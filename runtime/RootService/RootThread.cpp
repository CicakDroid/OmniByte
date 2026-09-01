// RootThread — Root service IPC wrapper.
// Source: https://github.com/MMRLApp/RootThread
// Commit: main branch, 2026-09-01
// License: GPL-3.0
//
// Provides C++ interface for root command execution via Unix domain socket
// or pipe, adapted from RootThread's Java-based IPC.

#include "RootThread.h"

#include <algorithm>
#include <atomic>
#include <cstring>
#include <thread>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <fcntl.h>

namespace omnibyte::root {

static constexpr const char* SOCKET_PATH = "/data/local/tmp/omnibyte_root.sock";

RootThread::~RootThread() {
    close();
}

bool RootThread::init() {
    // Try to connect to existing root service
    socketFd_ = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socketFd_ < 0) return false;

    struct sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (connect(socketFd_, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) == 0) {
        connected_ = true;
        return true;
    }

    // Fallback: use direct su pipe
    close(socketFd_);
    socketFd_ = -1;
    connected_ = false;
    return false;
}

RootResponse RootThread::execSync(const std::string& command,
                                   const std::vector<std::string>& args,
                                   int timeoutMs) {
    RootResponse response;
    (void)timeoutMs;

    if (!connected_) {
        // Fallback: execute via popen with su
        std::string fullCmd = "su -c '" + command;
        for (const auto& arg : args) {
            fullCmd += " " + arg;
        }
        fullCmd += "'";

        FILE* pipe = popen(fullCmd.c_str(), "r");
        if (!pipe) return response;

        char buf[4096];
        while (fgets(buf, sizeof(buf), pipe)) {
            response.stdout += buf;
        }
        response.exitCode = pclose(pipe);
        response.success = (response.exitCode == 0);
        return response;
    }

    // Socket-based execution
    std::string payload = command;
    for (const auto& arg : args) {
        payload += "\n" + arg;
    }

    ssize_t sent = write(socketFd_, payload.c_str(), payload.size());
    if (sent < 0) return response;

    // Read response
    char buf[8192] = {};
    ssize_t received = read(socketFd_, buf, sizeof(buf) - 1);
    if (received > 0) {
        response.stdout = std::string(buf, received);
        response.success = true;
        response.exitCode = 0;
    }

    return response;
}

void RootThread::execAsync(const RootRequest& request,
                            std::function<void(const RootResponse&)> callback) {
    std::thread([this, request, callback]() {
        auto response = execSync(request.command, request.arguments, request.timeoutMs);
        callback(response);
    }).detach();
}

bool RootThread::isConnected() const {
    return connected_;
}

void RootThread::close() {
    if (socketFd_ >= 0) {
        ::close(socketFd_);
        socketFd_ = -1;
    }
    connected_ = false;
}

}  // namespace omnibyte::root
