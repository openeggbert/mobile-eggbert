// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Net/Sockets/UdpClient.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/Net/Sockets/SocketException.hpp"
#include "System/Net/Sockets/detail/ErrnoTranslation.hpp"
#include <cstdio>

#if defined(_WIN32)
#  include <winsock2.h>
#  include <ws2tcpip.h>
#  if defined(_MSC_VER)
#    pragma comment(lib, "ws2_32.lib")
#  endif
#  include <mutex>
namespace {
    using SockFd = SOCKET;
    static const SockFd kBad = INVALID_SOCKET;
    inline int    toFd(SockFd s)  { return static_cast<int>(s); }
    inline SockFd toSk(int fd)    { return static_cast<SockFd>(fd); }
    inline void   closeSk(int fd) { ::closesocket(toSk(fd)); }
    inline bool   validFd(int fd) { return toSk(fd) != INVALID_SOCKET; }
    inline int lastErrorCode() { return WSAGetLastError(); }
    inline std::string netErr() {
        char buf[32]; snprintf(buf, sizeof(buf), "WSA error %d", WSAGetLastError()); return buf;
    }
    inline std::string gaErr(int /*rc*/) { return netErr(); }
    // Winsock error codes already share SocketError's own numbering (see SocketError.hpp's
    // doc comment) -- no translation needed here.
    inline System::Net::Sockets::SocketError toSocketError(int code) {
        return static_cast<System::Net::Sockets::SocketError>(code);
    }
    void wsaInit() {
        static std::once_flag f;
        std::call_once(f, []{ WSADATA d; WSAStartup(MAKEWORD(2,2), &d); });
    }
}
#elif defined(__EMSCRIPTEN__)
#  include "System/PlatformNotSupportedException.hpp"
namespace {
    inline bool validFd(int fd) { return fd >= 0; }
    inline void closeSk([[maybe_unused]] int fd) {}
}
#else
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <arpa/inet.h>
#  include <netdb.h>
#  include <unistd.h>
#  include <cerrno>
#  include <cstring>
namespace {
    using SockFd = int;
    static const SockFd kBad = -1;
    inline int    toFd(SockFd s)  { return s; }
    inline SockFd toSk(int fd)    { return fd; }
    inline void   closeSk(int fd) { ::close(fd); }
    inline bool   validFd(int fd) { return fd >= 0; }
    inline int lastErrorCode()           { return errno; }
    inline std::string netErr()          { return std::strerror(errno); }
    inline std::string gaErr(int rc)     { return ::gai_strerror(rc); }
    inline void wsaInit() {}
    // Verified against SocketErrorPal.Unix.cs's GetSocketErrorForNativeError: real .NET
    // translates POSIX errno into the WSA-numbered SocketError space before constructing a
    // SocketException. This port previously cast the raw POSIX errno straight into
    // SocketException's "errorCode" parameter unchanged, so SocketErrorCode never matched any
    // real SocketError value.
    inline System::Net::Sockets::SocketError toSocketError(int code) {
        return SharpRuntimeDetail::Net::Sockets::TranslateErrno(code);
    }
}
#endif

namespace System::Net::Sockets {

#if !defined(__EMSCRIPTEN__)
static int makeUdpSocket() {
    wsaInit();
    SockFd fd = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == kBad)
        throw SocketException(toSocketError(lastErrorCode()), "UdpClient: socket() failed: " + netErr());
    return toFd(fd);
}
#endif

UdpClient::UdpClient() {
#if defined(__EMSCRIPTEN__)
    throw System::PlatformNotSupportedException("UdpClient is not supported on Emscripten.");
#else
    fd_ = makeUdpSocket();
#endif
}

UdpClient::UdpClient(int port) {
#if defined(__EMSCRIPTEN__)
    (void)port;
    throw System::PlatformNotSupportedException("UdpClient is not supported on Emscripten.");
#else
    fd_ = makeUdpSocket();
    struct sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = ::htons(static_cast<uint16_t>(port));
    if (::bind(toSk(fd_), reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        auto code = lastErrorCode();
        auto err = netErr();
        closeSk(fd_); fd_ = -1;
        throw SocketException(toSocketError(code), "UdpClient: bind() failed: " + err);
    }
#endif
}

UdpClient::UdpClient(const Net::IPEndPoint& localEP) {
#if defined(__EMSCRIPTEN__)
    (void)localEP;
    throw System::PlatformNotSupportedException("UdpClient is not supported on Emscripten.");
#else
    fd_ = makeUdpSocket();
    struct sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = ::htonl(localEP.getAddressProperty().getAddressProperty());
    addr.sin_port        = ::htons(static_cast<uint16_t>(localEP.getPortProperty()));
    if (::bind(toSk(fd_), reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        auto code = lastErrorCode();
        auto err = netErr();
        closeSk(fd_); fd_ = -1;
        throw SocketException(toSocketError(code), "UdpClient: bind() failed: " + err);
    }
#endif
}

UdpClient::~UdpClient() { Close(); }

void UdpClient::Connect(const std::string& hostname, int port) {
#if defined(__EMSCRIPTEN__)
    (void)hostname; (void)port;
    throw System::PlatformNotSupportedException("UdpClient is not supported on Emscripten.");
#else
    struct addrinfo hints{}, *res = nullptr;
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    auto portStr = std::to_string(port);
    int rc = ::getaddrinfo(hostname.c_str(), portStr.c_str(), &hints, &res);
    if (rc != 0)
        throw SocketException(SocketError::HostNotFound, "UdpClient::Connect: DNS failed: " + gaErr(rc));

    auto* sa = reinterpret_cast<struct sockaddr_in*>(res->ai_addr);
    remote_.setAddressProperty(Net::IPAddress(::ntohl(sa->sin_addr.s_addr)));
    remote_.setPortProperty(static_cast<SharpRuntime::intcs>(::ntohs(sa->sin_port)));
    ::freeaddrinfo(res);

    struct sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = ::htonl(remote_.getAddressProperty().getAddressProperty());
    addr.sin_port        = ::htons(static_cast<uint16_t>(remote_.getPortProperty()));
    if (::connect(toSk(fd_), reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0)
        throw SocketException(toSocketError(lastErrorCode()), "UdpClient::Connect: connect() failed: " + netErr());
    hasRemote_ = true;
#endif
}

void UdpClient::Connect(const Net::IPEndPoint& remoteEP) {
#if defined(__EMSCRIPTEN__)
    (void)remoteEP;
    throw System::PlatformNotSupportedException("UdpClient is not supported on Emscripten.");
#else
    remote_ = remoteEP;
    struct sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = ::htonl(remoteEP.getAddressProperty().getAddressProperty());
    addr.sin_port        = ::htons(static_cast<uint16_t>(remoteEP.getPortProperty()));
    if (::connect(toSk(fd_), reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0)
        throw SocketException(toSocketError(lastErrorCode()), "UdpClient::Connect: connect() failed: " + netErr());
    hasRemote_ = true;
#endif
}

int UdpClient::Send(const std::vector<SharpRuntime::bytecs>& dgram, int bytes) {
#if defined(__EMSCRIPTEN__)
    (void)dgram; (void)bytes;
    throw System::PlatformNotSupportedException("UdpClient is not supported on Emscripten.");
#else
    // Real .NET's UdpClient.Send(byte[], int, ...) routes through Socket.Send's
    // ValidateBufferArguments, which rejects a `bytes` count outside [0, dgram.Length]. This
    // port previously cast `bytes` straight to size_t with no check at all -- confirmed via a
    // standalone ASan repro that an oversized (or negative, wrapping to huge) count reads past
    // the end of `dgram` (heap-buffer-overflow).
    if (static_cast<uint32_t>(bytes) > dgram.size())
        throw System::ArgumentOutOfRangeException("bytes");
    if (!hasRemote_)
        throw System::InvalidOperationException("UdpClient::Send: must call Connect() before Send().");
    auto n = ::send(toSk(fd_), reinterpret_cast<const char*>(dgram.data()),
                    static_cast<size_t>(bytes), 0);
    if (n < 0)
        throw SocketException(toSocketError(lastErrorCode()), "UdpClient::Send: send() failed: " + netErr());
    return static_cast<int>(n);
#endif
}

std::vector<SharpRuntime::bytecs> UdpClient::Receive(Net::IPEndPoint& remoteEP) {
#if defined(__EMSCRIPTEN__)
    (void)remoteEP;
    throw System::PlatformNotSupportedException("UdpClient is not supported on Emscripten.");
#else
    std::vector<SharpRuntime::bytecs> buf(65536);
    struct sockaddr_in sender{};
    socklen_t len = sizeof(sender);
    auto n = ::recvfrom(toSk(fd_), reinterpret_cast<char*>(buf.data()), buf.size(), 0,
                        reinterpret_cast<struct sockaddr*>(&sender), &len);
    if (n < 0)
        throw SocketException(toSocketError(lastErrorCode()), "UdpClient::Receive: recvfrom() failed: " + netErr());
    remoteEP.setAddressProperty(Net::IPAddress(::ntohl(sender.sin_addr.s_addr)));
    remoteEP.setPortProperty(static_cast<SharpRuntime::intcs>(::ntohs(sender.sin_port)));
    buf.resize(static_cast<size_t>(n));
    return buf;
#endif
}

void UdpClient::Close() {
    if (validFd(fd_)) {
        closeSk(fd_);
        fd_        = -1;
        hasRemote_ = false;
    }
}

} // namespace System::Net::Sockets
