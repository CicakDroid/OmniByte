#pragma once
// NetworkPacketHookAdapter — Native network function hooking backend.
// Source: https://github.com/CicakDroid/OmniByte (custom implementation)
// Version: 1.0.0
//
// Bridges IHookBackend interface to libc.so network function hooking.
// Intercepts send(), recv(), read(), write(), connect(), socket() for raw TCP/UDP capture.
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
// ponytail: Uses ShadowhookAdapter for inline hooking, no external dependency.

#include "../../IHookEngines.h"

#include <string>
#include <unordered_map>
#include <mutex>
#include <functional>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace omnibyte::runtime::backends {

/// Native network function hook backend for intercepting socket operations.
/// Hooks libc.so network functions to monitor/modify TCP/UDP traffic.
class NetworkPacketHookAdapter : public IHookBackend {
public:
    NetworkPacketHookAdapter() = default;
    ~NetworkPacketHookAdapter() override;

    std::string name() const override { return "NPhook"; }
    bool isAvailable() const override;

    /// Hook a function at `addr` by rewriting its prologue.
    bool hookFunction(uintptr_t addr, void* replacement, void** originalOut) override;

    /// Unhook a previously hooked function, restoring original bytes.
    bool unhook(uintptr_t addr) override;

    /// Raw memory patch (delegates to KittyMemory fallback).
    bool patchMemory(uintptr_t addr, const uint8_t* data, size_t size) override;

    /// Initialize NPhook adapter. Must be called before any hook operations.
    /// Uses ShadowhookAdapter internally for inline hooking.
    bool init(bool debug = false);

    // --- Network-specific API (beyond IHookBackend) ---

    /// Hook send() to intercept outgoing data.
    /// callback: called with fd, buffer, length, flags, return value.
    bool hookSend(std::function<void(int fd, const void* buf, size_t len, int flags, ssize_t ret)> callback);

    /// Hook recv() to intercept incoming data.
    /// callback: called with fd, buffer, length, flags, return value.
    bool hookRecv(std::function<void(int fd, void* buf, size_t len, int flags, ssize_t ret)> callback);

    /// Hook read() to intercept incoming data (alias for recv on sockets).
    /// callback: called with fd, buffer, length, return value.
    bool hookRead(std::function<void(int fd, void* buf, size_t len, ssize_t ret)> callback);

    /// Hook write() to intercept outgoing data (alias for send on sockets).
    /// callback: called with fd, buffer, length, return value.
    bool hookWrite(std::function<void(int fd, const void* buf, size_t len, ssize_t ret)> callback);

    /// Hook connect() to intercept socket connections.
    /// callback: called with fd, destination address, port, return value.
    bool hookConnect(std::function<void(int fd, const struct sockaddr* addr, socklen_t addrlen, int ret)> callback);

    /// Hook socket() to intercept socket creation.
    /// callback: called with domain, type, protocol, return value.
    bool hookSocket(std::function<void(int domain, int type, int protocol, int ret)> callback);

    /// Hook all network functions at once.
    bool hookAll();

    /// Unhook all network functions.
    void unhookAll();

    /// Get NPhook version string.
    static const char* getVersion();

    /// Check if NPhook is initialized.
    bool isInitialized() const { return initialized_; }

    /// Get last init error code.
    int getInitError() const { return initError_; }

private:

    bool initialized_ = false;
    int initError_ = -1;

    // Hook registry: address → stub for unhook
    std::unordered_map<uintptr_t, void*> hooks_;
    std::mutex hooksMutex_;

    // Original function pointers
    void* originalSend_ = nullptr;
    void* originalRecv_ = nullptr;
    void* originalRead_ = nullptr;
    void* originalWrite_ = nullptr;
    void* originalConnect_ = nullptr;
    void* originalSocket_ = nullptr;

    // Callback storage
    std::function<void(int fd, const void* buf, size_t len, int flags, ssize_t ret)> onSend_;
    std::function<void(int fd, void* buf, size_t len, int flags, ssize_t ret)> onRecv_;
    std::function<void(int fd, void* buf, size_t len, ssize_t ret)> onRead_;
    std::function<void(int fd, const void* buf, size_t len, ssize_t ret)> onWrite_;
    std::function<void(int fd, const struct sockaddr* addr, socklen_t addrlen, int ret)> onConnect_;
    std::function<void(int domain, int type, int protocol, int ret)> onSocket_;
};

} // namespace omnibyte::runtime::backends
