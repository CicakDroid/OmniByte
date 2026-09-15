#pragma once
// NPHooking — Abstraction layer for network packet hooking process.
// Source: https://github.com/CicakDroid/OmniByte (custom implementation)
// Version: 1.0.0
//
// Provides high-level network hooking operations using NetworkPacketHookAdapter.
// Supports TCP/UDP capture, connection tracking, and packet interception.
//
// References (verified citations):
//   - sowmiksudo/android-tcp-sniffer — Frida-based libc.so hooking for raw TCP capture
//     https://github.com/sowmiksudo/android-tcp-sniffer (MIT)
//     Hooks send, recv, read, write to capture raw TCP payloads.
//   - FrenchYeti/interruptor — System call tracing library based on Frida's Stalker
//     https://github.com/frenchyeti/interruptor
//     Human-friendly cross-platform syscall tracing and hooking.
//   - iddoeldor/frida-snippets — Frida snippets including socket activity hooks
//     https://github.com/iddoeldor/frida-snippets
//     Interceptor.attach on connect, recv, send, read, write in libc.so.
//
// ponytail: Minimal abstraction, direct adapter usage.

#include "NetworkPackethook/NetworkPacketHookAdapter.h"

#include <string>
#include <functional>
#include <vector>
#include <memory>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace omnibyte::runtime::backends {

/// Network packet hooking process abstraction layer.
/// Provides high-level API for intercepting and modifying network operations.
class NPHooking {
public:
    NPHooking() = default;
    ~NPHooking() = default;

    /// Initialize network hooking with specified adapter.
    bool init(std::shared_ptr<NetworkPacketHookAdapter> adapter);

    /// Hook all network functions for monitoring.
    bool hookAll();

    /// Unhook all network functions.
    void unhookAll();

    /// Hook specific function: send().
    bool hookSend(std::function<void(int fd, const void* buf, size_t len, int flags, ssize_t ret)> callback);

    /// Hook specific function: recv().
    bool hookRecv(std::function<void(int fd, void* buf, size_t len, int flags, ssize_t ret)> callback);

    /// Hook specific function: read().
    bool hookRead(std::function<void(int fd, void* buf, size_t len, ssize_t ret)> callback);

    /// Hook specific function: write().
    bool hookWrite(std::function<void(int fd, const void* buf, size_t len, ssize_t ret)> callback);

    /// Hook specific function: connect().
    bool hookConnect(std::function<void(int fd, const struct sockaddr* addr, socklen_t addrlen, int ret)> callback);

    /// Hook specific function: socket().
    bool hookSocket(std::function<void(int domain, int type, int protocol, int ret)> callback);

    /// Get list of all connected hosts (IP:port).
    std::vector<std::string> getConnectedHosts() const;

    /// Get list of all intercepted packet sizes.
    std::vector<size_t> getInterceptedPackets() const;

    /// Clear intercepted packets log.
    void clearInterceptedPackets();

    /// Check if network hooking is initialized.
    bool isInitialized() const { return initialized_; }

    /// Get adapter version.
    const char* getAdapterVersion() const;

private:
    bool initialized_ = false;
    std::shared_ptr<NetworkPacketHookAdapter> adapter_;

    // Connected hosts tracking
    std::vector<std::string> connectedHosts_;
    std::vector<size_t> interceptedPackets_;
};

} // namespace omnibyte::runtime::backends
