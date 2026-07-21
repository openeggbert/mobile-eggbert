// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Net/Sockets/UnixDomainSocketEndPoint.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Net/SocketAddress.hpp"
#include "System/PlatformNotSupportedException.hpp"

#if defined(_WIN32)
#include <winsock2.h>
#include <afunix.h>
#elif defined(__EMSCRIPTEN__)
// No AF_UNIX support under Emscripten.
#else
#include <sys/un.h>
#endif

namespace System::Net::Sockets {

#if !defined(__EMSCRIPTEN__)
namespace {
    constexpr size_t kNativePathLength = sizeof(sockaddr_un::sun_path); // 108 on Linux
} // namespace
#endif

UnixDomainSocketEndPoint::UnixDomainSocketEndPoint(const std::string& path) : path_(path) {
#if defined(__EMSCRIPTEN__)
    (void)path;
    throw System::PlatformNotSupportedException("Unix domain sockets are not supported on Emscripten.");
#else
    bool isAbstractPath = isAbstract(path);
    size_t bufferLength = path.size() + (isAbstractPath ? 0 : 1); // pathname addresses need a NUL terminator

    if (path.empty() || bufferLength > kNativePathLength) {
        throw System::ArgumentOutOfRangeException(
            "path", "The path is of an invalid length for use with domain sockets on this platform.");
    }
#endif
}

std::string UnixDomainSocketEndPoint::ToString() const {
    if (isAbstract(path_)) {
        return "@" + path_.substr(1);
    }
    return path_;
}

SocketAddress UnixDomainSocketEndPoint::Serialize() const {
    SocketAddress address(System::Net::Sockets::AddressFamily::Unix,
                           static_cast<SharpRuntime::intcs>(2 + path_.size()));
    for (size_t i = 0; i < path_.size(); ++i) {
        address[static_cast<SharpRuntime::intcs>(2 + i)] = static_cast<SharpRuntime::bytecs>(path_[i]);
    }
    return address;
}

std::shared_ptr<System::Net::EndPoint> UnixDomainSocketEndPoint::Create(const SocketAddress& socketAddress) const {
    SharpRuntime::intcs pathLength = socketAddress.getSizeProperty() - 2;
    std::string path;
    if (pathLength > 0) {
        path.resize(static_cast<size_t>(pathLength));
        for (SharpRuntime::intcs i = 0; i < pathLength; ++i) {
            path[static_cast<size_t>(i)] = static_cast<char>(socketAddress[2 + i]);
        }
    }
    return std::make_shared<UnixDomainSocketEndPoint>(path, UnvalidatedTag{});
}

} // namespace System::Net::Sockets
