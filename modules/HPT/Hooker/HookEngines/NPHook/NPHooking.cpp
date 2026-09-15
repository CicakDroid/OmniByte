// NPHooking.cpp — Implementation for network packet hooking process abstraction.
// Source: https://github.com/CicakDroid/OmniByte (custom implementation)
// License: MIT
// Version: 1.0.0
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

#include "NPHooking.h"
#include <android/log.h>
#include <cstring>
#include <arpa/inet.h>

#define TAG "NPHooking"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

namespace omnibyte::runtime::backends {

bool NPHooking::init(std::shared_ptr<NetworkPacketHookAdapter> adapter) {
    if (!adapter) {
        LOGE("Null adapter provided");
        return false;
    }

    adapter_ = adapter;
    initialized_ = true;
    LOGI("NPHooking initialized with adapter: %s", adapter_->name().c_str());
    return true;
}

bool NPHooking::hookAll() {
    if (!initialized_ || !adapter_) {
        LOGE("NPHooking not initialized");
        return false;
    }

    // Hook all network functions
    adapter_->hookAll();
    LOGI("All network functions hooked");
    return true;
}

void NPHooking::unhookAll() {
    if (!initialized_ || !adapter_) {
        LOGE("NPHooking not initialized");
        return;
    }

    adapter_->unhookAll();
    connectedHosts_.clear();
    interceptedPackets_.clear();
    LOGI("All network hooks removed");
}

bool NPHooking::hookSend(std::function<void(int fd, const void* buf, size_t len, int flags, ssize_t ret)> callback) {
    if (!initialized_ || !adapter_) {
        LOGE("NPHooking not initialized");
        return false;
    }

    // Wrap callback to track intercepted packets
    auto wrappedCallback = [this, callback](int fd, const void* buf, size_t len, int flags, ssize_t ret) {
        if (ret > 0) {
            interceptedPackets_.push_back(ret);
            LOGI("send() fd=%d len=%zd ret=%zd", fd, len, ret);
        }
        callback(fd, buf, len, flags, ret);
    };

    return adapter_->hookSend(wrappedCallback);
}

bool NPHooking::hookRecv(std::function<void(int fd, void* buf, size_t len, int flags, ssize_t ret)> callback) {
    if (!initialized_ || !adapter_) {
        LOGE("NPHooking not initialized");
        return false;
    }

    // Wrap callback to track intercepted packets
    auto wrappedCallback = [this, callback](int fd, void* buf, size_t len, int flags, ssize_t ret) {
        if (ret > 0) {
            interceptedPackets_.push_back(ret);
            LOGI("recv() fd=%d len=%zd ret=%zd", fd, len, ret);
        }
        callback(fd, buf, len, flags, ret);
    };

    return adapter_->hookRecv(wrappedCallback);
}

bool NPHooking::hookRead(std::function<void(int fd, void* buf, size_t len, ssize_t ret)> callback) {
    if (!initialized_ || !adapter_) {
        LOGE("NPHooking not initialized");
        return false;
    }

    return adapter_->hookRead(callback);
}

bool NPHooking::hookWrite(std::function<void(int fd, const void* buf, size_t len, ssize_t ret)> callback) {
    if (!initialized_ || !adapter_) {
        LOGE("NPHooking not initialized");
        return false;
    }

    return adapter_->hookWrite(callback);
}

bool NPHooking::hookConnect(std::function<void(int fd, const struct sockaddr* addr, socklen_t addrlen, int ret)> callback) {
    if (!initialized_ || !adapter_) {
        LOGE("NPHooking not initialized");
        return false;
    }

    // Wrap callback to track connected hosts
    auto wrappedCallback = [this, callback](int fd, const struct sockaddr* addr, socklen_t addrlen, int ret) {
        if (addr && ret == 0) {
            char ip[INET6_ADDRSTRLEN] = {0};
            uint16_t port = 0;

            if (addr->sa_family == AF_INET) {
                auto* addr4 = reinterpret_cast<const struct sockaddr_in*>(addr);
                inet_ntop(AF_INET, &addr4->sin_addr, ip, sizeof(ip));
                port = ntohs(addr4->sin_port);
            } else if (addr->sa_family == AF_INET6) {
                auto* addr6 = reinterpret_cast<const struct sockaddr_in6*>(addr);
                inet_ntop(AF_INET6, &addr6->sin6_addr, ip, sizeof(ip));
                port = ntohs(addr6->sin6_port);
            }

            if (ip[0] != '\0') {
                std::string host = std::string(ip) + ":" + std::to_string(port);
                connectedHosts_.push_back(host);
                LOGI("connect() fd=%d -> %s", fd, host.c_str());
            }
        }
        callback(fd, addr, addrlen, ret);
    };

    return adapter_->hookConnect(wrappedCallback);
}

bool NPHooking::hookSocket(std::function<void(int domain, int type, int protocol, int ret)> callback) {
    if (!initialized_ || !adapter_) {
        LOGE("NPHooking not initialized");
        return false;
    }

    return adapter_->hookSocket(callback);
}

std::vector<std::string> NPHooking::getConnectedHosts() const {
    return connectedHosts_;
}

std::vector<size_t> NPHooking::getInterceptedPackets() const {
    return interceptedPackets_;
}

void NPHooking::clearInterceptedPackets() {
    interceptedPackets_.clear();
    LOGI("Intercepted packets cleared");
}

const char* NPHooking::getAdapterVersion() const {
    if (!initialized_ || !adapter_) {
        return "not initialized";
    }
    return NetworkPacketHookAdapter::getVersion();
}

} // namespace omnibyte::runtime::backends
