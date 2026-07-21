// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Net/Sockets/Socket.hpp"
#include <array>
#include <cstring>
#include "System/Net/IPEndPoint.hpp"
#include "System/Net/Sockets/SocketException.hpp"
#include "System/Net/Sockets/UnixDomainSocketEndPoint.hpp"
#include "System/Net/Sockets/detail/ErrnoTranslation.hpp"
#include "System/Net/Dns.hpp"
#include "System/PlatformNotSupportedException.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"

#if defined(_WIN32)
#  include <winsock2.h>
#  include <ws2tcpip.h>
#  include <afunix.h>
#  include <mutex>
#  if defined(_MSC_VER)
#    pragma comment(lib, "ws2_32.lib")
#  endif
namespace {
    using SockFd = SOCKET;
    constexpr SockFd kBad = INVALID_SOCKET;
    inline int  toFd(SockFd s)  { return static_cast<int>(s); }
    inline SockFd toSk(int fd)    { return static_cast<SockFd>(fd); }
    inline void closeSk(int fd) { ::closesocket(toSk(fd)); }
    inline bool validFd(int fd) { return toSk(fd) != INVALID_SOCKET; }
    inline int lastError() { return WSAGetLastError(); }
    void wsaInit() {
        static std::once_flag f;
        std::call_once(f, [] { WSADATA d; WSAStartup(MAKEWORD(2, 2), &d); });
    }
    using OptLen = int;
}
#elif defined(__EMSCRIPTEN__)
namespace { /* no platform sockets helpers on Emscripten */ }
#else
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <netinet/tcp.h>
#  include <arpa/inet.h>
#  include <netdb.h>
#  include <unistd.h>
#  include <fcntl.h>
#  include <sys/ioctl.h>
#  include <sys/un.h>
#  include <cerrno>
namespace {
    using SockFd = int;
    constexpr SockFd kBad = -1;
    inline int  toFd(SockFd s)  { return s; }
    inline SockFd toSk(int fd)    { return fd; }
    inline void closeSk(int fd) { ::close(fd); }
    inline bool validFd(int fd) { return fd >= 0; }
    inline int lastError() { return errno; }
    inline void wsaInit() {}
    using OptLen = socklen_t;
}
#endif

namespace System::Net::Sockets {

using SharpRuntime::bytecs;
using SharpRuntime::intcs;
using SharpRuntime::longcs;

#if !defined(__EMSCRIPTEN__)

namespace {

    int nativeFamily(AddressFamily family) {
        switch (family) {
            case AddressFamily::InterNetwork: return AF_INET;
            case AddressFamily::InterNetworkV6: return AF_INET6;
            case AddressFamily::Unix: return AF_UNIX;
            default: return AF_UNSPEC;
        }
    }

    int nativeSockType(SocketType type) {
        switch (type) {
            case SocketType::Stream: return SOCK_STREAM;
            case SocketType::Dgram: return SOCK_DGRAM;
            case SocketType::Raw: return SOCK_RAW;
            case SocketType::Rdm:
#if defined(SOCK_RDM)
                return SOCK_RDM;
#else
                return SOCK_DGRAM;
#endif
            case SocketType::Seqpacket: return SOCK_SEQPACKET;
            default: return SOCK_STREAM;
        }
    }

    int nativeSocketFlags(SocketFlags flags) {
#if defined(_WIN32)
        // Winsock2's MSG_* flag bit values are numerically identical to this port's
        // System::Net::Sockets::SocketFlags (real .NET's own SocketFlags enum was itself
        // modeled on Winsock2) -- a direct cast is correct here.
        return static_cast<int>(flags);
#else
        // Verified against pal_networking.c's ConvertSocketFlagsPalToPlatform: real .NET only
        // ever passes OutOfBand/Peek/DontRoute as *input* flags to Send/Receive/SendTo/
        // ReceiveFrom -- Truncated/ControlDataTruncated/Broadcast/Multicast/Partial are
        // receive-*result*-only flags, never meaningful as input -- and translates each
        // meaningful input flag to its own POSIX MSG_* constant individually rather than
        // passing the bit pattern through, since .NET's SocketFlags bit positions only match
        // Linux's MSG_* constants by coincidence for these three flags. A raw
        // static_cast<int>(flags), this port's previous behavior, silently mapped unrelated
        // bits onto the wrong native flag whenever a caller passed anything else -- e.g.
        // SocketFlags::Truncated (0x0100) would have set Linux's MSG_WAITALL (also 0x100).
        int native = 0;
        if ((flags & SocketFlags::OutOfBand) != SocketFlags::None) native |= MSG_OOB;
        if ((flags & SocketFlags::Peek) != SocketFlags::None) native |= MSG_PEEK;
        if ((flags & SocketFlags::DontRoute) != SocketFlags::None) native |= MSG_DONTROUTE;
        return native;
#endif
    }

    // Builds a native sockaddr for `ep` into `storage`, returning the address length.
    // Supports IPEndPoint (v4/v6) and UnixDomainSocketEndPoint.
    socklen_t buildNativeAddress(const System::Net::EndPoint& ep, sockaddr_storage& storage) {
        std::memset(&storage, 0, sizeof(storage));

        if (const auto* ip = dynamic_cast<const System::Net::IPEndPoint*>(&ep)) {
            const System::Net::IPAddress& addr = ip->getAddressProperty();
            if (addr.getIsIPv6Property()) {
                auto* a6 = reinterpret_cast<sockaddr_in6*>(&storage);
                a6->sin6_family = AF_INET6;
                a6->sin6_port = ::htons(static_cast<uint16_t>(ip->getPortProperty()));
                auto bytes = addr.GetAddressBytes();
                std::memcpy(&a6->sin6_addr, bytes.data(), bytes.size());
                a6->sin6_scope_id = static_cast<uint32_t>(addr.getScopeIdProperty());
                return sizeof(sockaddr_in6);
            }
            auto* a4 = reinterpret_cast<sockaddr_in*>(&storage);
            a4->sin_family = AF_INET;
            a4->sin_port = ::htons(static_cast<uint16_t>(ip->getPortProperty()));
            a4->sin_addr.s_addr = ::htonl(addr.getAddressProperty());
            return sizeof(sockaddr_in);
        }

        if (const auto* uds = dynamic_cast<const UnixDomainSocketEndPoint*>(&ep)) {
            auto* au = reinterpret_cast<sockaddr_un*>(&storage);
            au->sun_family = AF_UNIX;
            const std::string& path = uds->getPathProperty();
            std::memcpy(au->sun_path, path.data(), std::min(path.size(), sizeof(au->sun_path) - 1));
            return static_cast<socklen_t>(offsetof(sockaddr_un, sun_path) + path.size() + 1);
        }

        throw System::ArgumentException("Unsupported EndPoint type for this runtime's Socket implementation.",
                                         "remoteEP");
    }

    std::shared_ptr<System::Net::EndPoint> endpointFromNative(const sockaddr_storage& storage) {
        if (storage.ss_family == AF_INET) {
            const auto* a4 = reinterpret_cast<const sockaddr_in*>(&storage);
            System::Net::IPAddress addr(static_cast<uint32_t>(::ntohl(a4->sin_addr.s_addr)));
            return std::make_shared<System::Net::IPEndPoint>(addr, static_cast<intcs>(::ntohs(a4->sin_port)));
        }
        if (storage.ss_family == AF_INET6) {
            const auto* a6 = reinterpret_cast<const sockaddr_in6*>(&storage);
            std::array<bytecs, 16> bytes{};
            std::memcpy(bytes.data(), &a6->sin6_addr, 16);
            System::Net::IPAddress addr(bytes, static_cast<longcs>(a6->sin6_scope_id));
            return std::make_shared<System::Net::IPEndPoint>(addr, static_cast<intcs>(::ntohs(a6->sin6_port)));
        }
        if (storage.ss_family == AF_UNIX) {
            const auto* au = reinterpret_cast<const sockaddr_un*>(&storage);
            std::string path(au->sun_path);
            if (path.empty()) {
                return nullptr;
            }
            return std::make_shared<UnixDomainSocketEndPoint>(path);
        }
        return nullptr;
    }

    [[noreturn]] void throwSocketError(const char* what) {
        int err = lastError();
#if defined(_WIN32)
        // Winsock error codes (WSAGetLastError()) already share SocketError's own numbering
        // (see SocketError.hpp's doc comment) -- no translation needed here.
        throw SocketException(static_cast<intcs>(err), what);
#elif defined(__EMSCRIPTEN__)
        throw SocketException(static_cast<intcs>(err), what);
#else
        throw SocketException(SharpRuntimeDetail::Net::Sockets::TranslateErrno(err), what);
#endif
    }

    // Verified against Socket.Tasks.cs's ValidateBufferArguments: casting to uint32_t before
    // comparing catches a negative offset/count as an out-of-range value too (a negative
    // int becomes a huge unsigned value), matching (uint)offset > (uint)buffer.Length and
    // (uint)size > (uint)(buffer.Length - offset) in the .NET source. Previously this port
    // did buffer.data() + offset with no bounds check at all -- a genuine out-of-bounds
    // heap read (Send/SendTo) or write (Receive/ReceiveFrom) when offset+count exceeded the
    // buffer, not just a parity gap.
    template<typename Buf>
    void validateBufferArgs(const Buf& buffer, intcs offset, intcs count) {
        uint32_t size = static_cast<uint32_t>(buffer.size());
        if (static_cast<uint32_t>(offset) > size)
            throw System::ArgumentOutOfRangeException("offset");
        if (static_cast<uint32_t>(count) > size - static_cast<uint32_t>(offset))
            throw System::ArgumentOutOfRangeException("count");
    }

} // namespace

#endif // !__EMSCRIPTEN__

Socket::Socket(AddressFamily addressFamily, SocketType socketType, ProtocolType protocolType)
    : addressFamily_(addressFamily), socketType_(socketType), protocolType_(protocolType) {
#if defined(__EMSCRIPTEN__)
    throw System::PlatformNotSupportedException("Socket is not supported on Emscripten.");
#else
    wsaInit();
    SockFd sock = ::socket(nativeFamily(addressFamily), nativeSockType(socketType), static_cast<int>(protocolType));
    if (sock == kBad) {
        throwSocketError("Socket::Socket: socket() failed");
    }
    fd_ = toFd(sock);
#endif
}

Socket::~Socket() { Close(); }

Socket::Socket(Socket&& other) noexcept
    : fd_(other.fd_), addressFamily_(other.addressFamily_), socketType_(other.socketType_),
      protocolType_(other.protocolType_), connected_(other.connected_), bound_(other.bound_),
      blocking_(other.blocking_) {
    other.fd_ = -1;
    other.connected_ = false;
    other.bound_ = false;
}

Socket& Socket::operator=(Socket&& other) noexcept {
    if (this != &other) {
        Close();
        fd_ = other.fd_;
        addressFamily_ = other.addressFamily_;
        socketType_ = other.socketType_;
        protocolType_ = other.protocolType_;
        connected_ = other.connected_;
        bound_ = other.bound_;
        blocking_ = other.blocking_;
        other.fd_ = -1;
        other.connected_ = false;
        other.bound_ = false;
    }
    return *this;
}

#if defined(__EMSCRIPTEN__)

std::shared_ptr<System::Net::EndPoint> Socket::getLocalEndPointProperty() const { return nullptr; }
std::shared_ptr<System::Net::EndPoint> Socket::getRemoteEndPointProperty() const { return nullptr; }
intcs Socket::getAvailableProperty() const { return 0; }
void Socket::setBlockingProperty(bool value) { blocking_ = value; }
intcs Socket::getReceiveTimeoutProperty() const { return 0; }
void Socket::setReceiveTimeoutProperty(intcs) {}
intcs Socket::getSendTimeoutProperty() const { return 0; }
void Socket::setSendTimeoutProperty(intcs) {}
intcs Socket::getReceiveBufferSizeProperty() const { return 0; }
void Socket::setReceiveBufferSizeProperty(intcs) {}
intcs Socket::getSendBufferSizeProperty() const { return 0; }
void Socket::setSendBufferSizeProperty(intcs) {}
bool Socket::getExclusiveAddressUseProperty() const { return false; }
void Socket::setExclusiveAddressUseProperty(bool) {}
bool Socket::getNoDelayProperty() const { return false; }
void Socket::setNoDelayProperty(bool) {}
intcs Socket::getTtlProperty() const { return 0; }
void Socket::setTtlProperty(intcs) {}
bool Socket::getDontFragmentProperty() const { return false; }
void Socket::setDontFragmentProperty(bool) {}
bool Socket::getMulticastLoopbackProperty() const { return false; }
void Socket::setMulticastLoopbackProperty(bool) {}
bool Socket::getEnableBroadcastProperty() const { return false; }
void Socket::setEnableBroadcastProperty(bool) {}
LingerOption Socket::getLingerStateProperty() const { return LingerOption(false, 0); }
void Socket::setLingerStateProperty(const LingerOption&) {}
void Socket::Bind(const System::Net::EndPoint&) {
    throw System::PlatformNotSupportedException("Socket is not supported on Emscripten.");
}
void Socket::Connect(const System::Net::EndPoint&) {
    throw System::PlatformNotSupportedException("Socket is not supported on Emscripten.");
}
void Socket::Connect(const System::Net::IPAddress&, intcs) {
    throw System::PlatformNotSupportedException("Socket is not supported on Emscripten.");
}
void Socket::Connect(const std::string&, intcs) {
    throw System::PlatformNotSupportedException("Socket is not supported on Emscripten.");
}
void Socket::Listen(intcs) { throw System::PlatformNotSupportedException("Socket is not supported on Emscripten."); }
void Socket::Listen() { throw System::PlatformNotSupportedException("Socket is not supported on Emscripten."); }
std::shared_ptr<Socket> Socket::Accept() {
    throw System::PlatformNotSupportedException("Socket is not supported on Emscripten.");
}
intcs Socket::Send(const std::vector<bytecs>&, intcs, intcs, SocketFlags) {
    throw System::PlatformNotSupportedException("Socket is not supported on Emscripten.");
}
intcs Socket::Send(const std::vector<bytecs>&, SocketFlags) {
    throw System::PlatformNotSupportedException("Socket is not supported on Emscripten.");
}
intcs Socket::Receive(std::vector<bytecs>&, intcs, intcs, SocketFlags) {
    throw System::PlatformNotSupportedException("Socket is not supported on Emscripten.");
}
intcs Socket::Receive(std::vector<bytecs>&, SocketFlags) {
    throw System::PlatformNotSupportedException("Socket is not supported on Emscripten.");
}
intcs Socket::SendTo(const std::vector<bytecs>&, intcs, intcs, SocketFlags, const System::Net::EndPoint&) {
    throw System::PlatformNotSupportedException("Socket is not supported on Emscripten.");
}
intcs Socket::SendTo(const std::vector<bytecs>&, const System::Net::EndPoint&) {
    throw System::PlatformNotSupportedException("Socket is not supported on Emscripten.");
}
SocketReceiveFromResult Socket::ReceiveFrom(std::vector<bytecs>&, intcs, intcs, SocketFlags,
                                             const System::Net::EndPoint&) {
    throw System::PlatformNotSupportedException("Socket is not supported on Emscripten.");
}
SocketReceiveFromResult Socket::ReceiveFrom(std::vector<bytecs>&, const System::Net::EndPoint&) {
    throw System::PlatformNotSupportedException("Socket is not supported on Emscripten.");
}
void Socket::Shutdown(SocketShutdown) {}
void Socket::Close() {}
bool Socket::Poll(longcs, SelectMode) const {
    throw System::PlatformNotSupportedException("Socket is not supported on Emscripten.");
}
void Socket::SetSocketOption(SocketOptionLevel, SocketOptionName, intcs) {}
void Socket::SetSocketOption(SocketOptionLevel, SocketOptionName, bool) {}
void Socket::SetSocketOption(SocketOptionLevel, SocketOptionName, const LingerOption&) {}
intcs Socket::GetSocketOption(SocketOptionLevel, SocketOptionName) const { return 0; }
bool Socket::getOSSupportsIPv4Property() { return false; }
bool Socket::getOSSupportsIPv6Property() { return false; }
bool Socket::getOSSupportsUnixDomainSocketsProperty() { return false; }

#else // POSIX / Windows

std::shared_ptr<System::Net::EndPoint> Socket::getLocalEndPointProperty() const {
    if (!validFd(fd_)) return nullptr;
    sockaddr_storage storage{};
    socklen_t len = sizeof(storage);
    if (::getsockname(toSk(fd_), reinterpret_cast<sockaddr*>(&storage), &len) != 0) return nullptr;
    return endpointFromNative(storage);
}

std::shared_ptr<System::Net::EndPoint> Socket::getRemoteEndPointProperty() const {
    if (!validFd(fd_) || !connected_) return nullptr;
    sockaddr_storage storage{};
    socklen_t len = sizeof(storage);
    if (::getpeername(toSk(fd_), reinterpret_cast<sockaddr*>(&storage), &len) != 0) return nullptr;
    return endpointFromNative(storage);
}

intcs Socket::getAvailableProperty() const {
    if (!validFd(fd_)) return 0;
#if defined(_WIN32)
    u_long n = 0;
    if (::ioctlsocket(toSk(fd_), FIONREAD, &n) != 0) throwSocketError("Socket::Available: ioctlsocket() failed");
#else
    int n = 0;
    if (::ioctl(fd_, FIONREAD, &n) != 0) throwSocketError("Socket::Available: ioctl() failed");
#endif
    return static_cast<intcs>(n);
}

void Socket::setBlockingProperty(bool value) {
    if (validFd(fd_)) {
#if defined(_WIN32)
        u_long mode = value ? 0 : 1;
        ::ioctlsocket(toSk(fd_), FIONBIO, &mode);
#else
        int flags = ::fcntl(fd_, F_GETFL, 0);
        if (value) flags &= ~O_NONBLOCK;
        else flags |= O_NONBLOCK;
        ::fcntl(fd_, F_SETFL, flags);
#endif
    }
    blocking_ = value;
}

namespace {

    template<typename T>
    T getOpt(intcs fd, int level, int name) {
        T value{};
        OptLen len = sizeof(value);
#if defined(_WIN32)
        ::getsockopt(toSk(fd), level, name, reinterpret_cast<char*>(&value), &len);
#else
        ::getsockopt(fd, level, name, &value, &len);
#endif
        return value;
    }

    template<typename T>
    void setOpt(intcs fd, int level, int name, const T& value) {
#if defined(_WIN32)
        if (::setsockopt(toSk(fd), level, name, reinterpret_cast<const char*>(&value), sizeof(value)) != 0)
#else
        if (::setsockopt(fd, level, name, &value, sizeof(value)) != 0)
#endif
        {
            throwSocketError("Socket::SetSocketOption: setsockopt() failed");
        }
    }

#if !defined(_WIN32)
    timeval millisecondsToTimeval(intcs ms) {
        timeval tv{};
        tv.tv_sec = ms / 1000;
        tv.tv_usec = (ms % 1000) * 1000;
        return tv;
    }
#endif

} // namespace

intcs Socket::getReceiveTimeoutProperty() const { return getOpt<int>(fd_, SOL_SOCKET, SO_RCVTIMEO); }
void Socket::setReceiveTimeoutProperty(intcs value) {
#if defined(_WIN32)
    setOpt(fd_, SOL_SOCKET, SO_RCVTIMEO, static_cast<DWORD>(value));
#else
    setOpt(fd_, SOL_SOCKET, SO_RCVTIMEO, millisecondsToTimeval(value));
#endif
}

intcs Socket::getSendTimeoutProperty() const { return getOpt<int>(fd_, SOL_SOCKET, SO_SNDTIMEO); }
void Socket::setSendTimeoutProperty(intcs value) {
#if defined(_WIN32)
    setOpt(fd_, SOL_SOCKET, SO_SNDTIMEO, static_cast<DWORD>(value));
#else
    setOpt(fd_, SOL_SOCKET, SO_SNDTIMEO, millisecondsToTimeval(value));
#endif
}

intcs Socket::getReceiveBufferSizeProperty() const { return getOpt<int>(fd_, SOL_SOCKET, SO_RCVBUF); }
void Socket::setReceiveBufferSizeProperty(intcs value) { setOpt(fd_, SOL_SOCKET, SO_RCVBUF, static_cast<int>(value)); }

intcs Socket::getSendBufferSizeProperty() const { return getOpt<int>(fd_, SOL_SOCKET, SO_SNDBUF); }
void Socket::setSendBufferSizeProperty(intcs value) { setOpt(fd_, SOL_SOCKET, SO_SNDBUF, static_cast<int>(value)); }

bool Socket::getExclusiveAddressUseProperty() const { return getOpt<int>(fd_, SOL_SOCKET, SO_REUSEADDR) == 0; }
void Socket::setExclusiveAddressUseProperty(bool value) {
    setOpt(fd_, SOL_SOCKET, SO_REUSEADDR, value ? 0 : 1);
}

bool Socket::getNoDelayProperty() const { return getOpt<int>(fd_, IPPROTO_TCP, TCP_NODELAY) != 0; }
void Socket::setNoDelayProperty(bool value) { setOpt(fd_, IPPROTO_TCP, TCP_NODELAY, value ? 1 : 0); }

intcs Socket::getTtlProperty() const {
    if (addressFamily_ == AddressFamily::InterNetworkV6) return getOpt<int>(fd_, IPPROTO_IPV6, IPV6_UNICAST_HOPS);
    return getOpt<int>(fd_, IPPROTO_IP, IP_TTL);
}
void Socket::setTtlProperty(intcs value) {
    if (addressFamily_ == AddressFamily::InterNetworkV6) setOpt(fd_, IPPROTO_IPV6, IPV6_UNICAST_HOPS, static_cast<int>(value));
    else setOpt(fd_, IPPROTO_IP, IP_TTL, static_cast<int>(value));
}

bool Socket::getDontFragmentProperty() const {
#if defined(IP_MTU_DISCOVER) && !defined(_WIN32)
    return getOpt<int>(fd_, IPPROTO_IP, IP_MTU_DISCOVER) == IP_PMTUDISC_DO;
#else
    return false;
#endif
}
void Socket::setDontFragmentProperty(bool value) {
#if defined(IP_MTU_DISCOVER) && !defined(_WIN32)
    setOpt(fd_, IPPROTO_IP, IP_MTU_DISCOVER, value ? IP_PMTUDISC_DO : IP_PMTUDISC_WANT);
#else
    (void)value;
#endif
}

bool Socket::getMulticastLoopbackProperty() const {
    if (addressFamily_ == AddressFamily::InterNetworkV6) return getOpt<int>(fd_, IPPROTO_IPV6, IPV6_MULTICAST_LOOP) != 0;
    return getOpt<SharpRuntime::bytecs>(fd_, IPPROTO_IP, IP_MULTICAST_LOOP) != 0;
}
void Socket::setMulticastLoopbackProperty(bool value) {
    if (addressFamily_ == AddressFamily::InterNetworkV6) setOpt(fd_, IPPROTO_IPV6, IPV6_MULTICAST_LOOP, value ? 1 : 0);
    else setOpt(fd_, IPPROTO_IP, IP_MULTICAST_LOOP, static_cast<SharpRuntime::bytecs>(value ? 1 : 0));
}

bool Socket::getEnableBroadcastProperty() const { return getOpt<int>(fd_, SOL_SOCKET, SO_BROADCAST) != 0; }
void Socket::setEnableBroadcastProperty(bool value) { setOpt(fd_, SOL_SOCKET, SO_BROADCAST, value ? 1 : 0); }

LingerOption Socket::getLingerStateProperty() const {
    linger l = getOpt<linger>(fd_, SOL_SOCKET, SO_LINGER);
    return LingerOption(l.l_onoff != 0, static_cast<intcs>(l.l_linger));
}
void Socket::setLingerStateProperty(const LingerOption& value) {
    linger l{};
    l.l_onoff = value.getEnabledProperty() ? 1 : 0;
    l.l_linger = static_cast<decltype(l.l_linger)>(value.getLingerTimeProperty());
    setOpt(fd_, SOL_SOCKET, SO_LINGER, l);
}

void Socket::Bind(const System::Net::EndPoint& localEP) {
    sockaddr_storage storage{};
    socklen_t len = buildNativeAddress(localEP, storage);
    if (::bind(toSk(fd_), reinterpret_cast<sockaddr*>(&storage), len) != 0) {
        throwSocketError("Socket::Bind: bind() failed");
    }
    bound_ = true;
}

void Socket::Connect(const System::Net::EndPoint& remoteEP) {
    sockaddr_storage storage{};
    socklen_t len = buildNativeAddress(remoteEP, storage);
    if (::connect(toSk(fd_), reinterpret_cast<sockaddr*>(&storage), len) != 0) {
        throwSocketError("Socket::Connect: connect() failed");
    }
    connected_ = true;
}

void Socket::Connect(const System::Net::IPAddress& address, intcs port) { Connect(System::Net::IPEndPoint(address, port)); }

void Socket::Connect(const std::string& host, intcs port) {
    auto addresses = System::Net::Dns::GetHostAddresses(host);
    if (addresses.empty()) {
        throw SocketException(SocketError::HostNotFound, "Socket::Connect: could not resolve host " + host);
    }

    // Verified against Socket.cs's Connect(string,int) -> Connect(IPAddress[],int): real .NET
    // tries every resolved address in order (skipping ones whose family doesn't match this
    // socket's, via CanTryAddressFamily -- DualMode isn't implemented in this port, so only the
    // exact-family half of that check applies) and only throws once every candidate has failed.
    // This port previously connected to addresses[0] only, so a host whose first DNS answer was
    // unreachable/refused failed outright even when a later answer (e.g. the IPv6 address before
    // a working IPv4 one) would have succeeded.
    bool triedAny = false;
    std::unique_ptr<SocketException> lastError;
    for (const auto& address : addresses) {
        if (address.getAddressFamilyProperty() != addressFamily_) continue;
        triedAny = true;
        try {
            Connect(System::Net::IPEndPoint(address, port));
            return;
        } catch (const SocketException& ex) {
            lastError = std::make_unique<SocketException>(ex);
        }
    }
    if (lastError) throw *lastError;
    if (!triedAny) {
        throw System::ArgumentException("Socket::Connect: no resolved address matches this socket's address family.", "host");
    }
}

void Socket::Listen(intcs backlog) {
    if (::listen(toSk(fd_), static_cast<int>(backlog)) != 0) {
        throwSocketError("Socket::Listen: listen() failed");
    }
}

void Socket::Listen() { Listen(static_cast<intcs>(SOMAXCONN)); }

std::shared_ptr<Socket> Socket::Accept() {
    sockaddr_storage storage{};
    socklen_t len = sizeof(storage);
    SockFd client = ::accept(toSk(fd_), reinterpret_cast<sockaddr*>(&storage), &len);
    if (client == kBad) {
        throwSocketError("Socket::Accept: accept() failed");
    }
    return std::shared_ptr<Socket>(new Socket(toFd(client), addressFamily_, socketType_, protocolType_));
}

intcs Socket::Send(const std::vector<bytecs>& buffer, intcs offset, intcs count, SocketFlags flags) {
    validateBufferArgs(buffer, offset, count);
    if (count == 0) return 0;
#if defined(_WIN32)
    int sent = ::send(toSk(fd_), reinterpret_cast<const char*>(buffer.data() + offset), count,
                       nativeSocketFlags(flags));
#else
    ssize_t sent = ::send(fd_, buffer.data() + offset, static_cast<size_t>(count), nativeSocketFlags(flags));
#endif
    if (sent < 0) {
        throwSocketError("Socket::Send: send() failed");
    }
    return static_cast<intcs>(sent);
}

intcs Socket::Send(const std::vector<bytecs>& buffer, SocketFlags flags) {
    return Send(buffer, 0, static_cast<intcs>(buffer.size()), flags);
}

intcs Socket::Receive(std::vector<bytecs>& buffer, intcs offset, intcs count, SocketFlags flags) {
    validateBufferArgs(buffer, offset, count);
    if (count == 0) return 0;
#if defined(_WIN32)
    int received = ::recv(toSk(fd_), reinterpret_cast<char*>(buffer.data() + offset), count,
                           nativeSocketFlags(flags));
#else
    ssize_t received = ::recv(fd_, buffer.data() + offset, static_cast<size_t>(count), nativeSocketFlags(flags));
#endif
    if (received < 0) {
        throwSocketError("Socket::Receive: recv() failed");
    }
    return static_cast<intcs>(received);
}

intcs Socket::Receive(std::vector<bytecs>& buffer, SocketFlags flags) {
    return Receive(buffer, 0, static_cast<intcs>(buffer.size()), flags);
}

intcs Socket::SendTo(const std::vector<bytecs>& buffer, intcs offset, intcs count, SocketFlags flags,
                      const System::Net::EndPoint& remoteEP) {
    validateBufferArgs(buffer, offset, count);
    sockaddr_storage storage{};
    socklen_t len = buildNativeAddress(remoteEP, storage);
#if defined(_WIN32)
    int sent = ::sendto(toSk(fd_), reinterpret_cast<const char*>(buffer.data() + offset), count,
                         nativeSocketFlags(flags), reinterpret_cast<sockaddr*>(&storage), static_cast<int>(len));
#else
    ssize_t sent = ::sendto(fd_, buffer.data() + offset, static_cast<size_t>(count), nativeSocketFlags(flags),
                             reinterpret_cast<sockaddr*>(&storage), len);
#endif
    if (sent < 0) {
        throwSocketError("Socket::SendTo: sendto() failed");
    }
    return static_cast<intcs>(sent);
}

intcs Socket::SendTo(const std::vector<bytecs>& buffer, const System::Net::EndPoint& remoteEP) {
    return SendTo(buffer, 0, static_cast<intcs>(buffer.size()), SocketFlags::None, remoteEP);
}

SocketReceiveFromResult Socket::ReceiveFrom(std::vector<bytecs>& buffer, intcs offset, intcs count, SocketFlags flags,
                                              const System::Net::EndPoint& /*remoteEPTemplate*/) {
    validateBufferArgs(buffer, offset, count);
    sockaddr_storage storage{};
    socklen_t len = sizeof(storage);
#if defined(_WIN32)
    int received = ::recvfrom(toSk(fd_), reinterpret_cast<char*>(buffer.data() + offset), count,
                               nativeSocketFlags(flags), reinterpret_cast<sockaddr*>(&storage), &len);
#else
    ssize_t received = ::recvfrom(fd_, buffer.data() + offset, static_cast<size_t>(count), nativeSocketFlags(flags),
                                   reinterpret_cast<sockaddr*>(&storage), &len);
#endif
    if (received < 0) {
        throwSocketError("Socket::ReceiveFrom: recvfrom() failed");
    }
    SocketReceiveFromResult result;
    result.ReceivedBytes = static_cast<intcs>(received);
    result.RemoteEndPoint = endpointFromNative(storage);
    return result;
}

SocketReceiveFromResult Socket::ReceiveFrom(std::vector<bytecs>& buffer, const System::Net::EndPoint& remoteEPTemplate) {
    return ReceiveFrom(buffer, 0, static_cast<intcs>(buffer.size()), SocketFlags::None, remoteEPTemplate);
}

void Socket::Shutdown(SocketShutdown how) {
    if (!validFd(fd_)) return;
#if defined(_WIN32)
    int nativeHow = how == SocketShutdown::Receive ? SD_RECEIVE : how == SocketShutdown::Send ? SD_SEND : SD_BOTH;
#else
    int nativeHow = how == SocketShutdown::Receive ? SHUT_RD : how == SocketShutdown::Send ? SHUT_WR : SHUT_RDWR;
#endif
    ::shutdown(toSk(fd_), nativeHow);
}

void Socket::Close() {
    if (validFd(fd_)) {
        closeSk(fd_);
        fd_ = -1;
        connected_ = false;
        bound_ = false;
    }
}

bool Socket::Poll(longcs microSeconds, SelectMode mode) const {
    if (!validFd(fd_)) return false;
    fd_set set;
    FD_ZERO(&set);
    FD_SET(toSk(fd_), &set);

    // Verified against SocketPal.Unix.cs's Poll: real .NET treats microSeconds == -1 as an
    // infinite wait ("milliseconds = microseconds == -1 ? -1 : microseconds / 1000", then
    // passed to native poll() with a -1 timeout meaning "block indefinitely"). This port
    // previously always built a timeval from microSeconds unconditionally: for -1, C++
    // truncating division/modulo gives tv_sec=0, tv_usec=-1 -- a timeval with a negative
    // tv_usec is invalid per POSIX, so select() failed with EINVAL and Poll(-1, ...) returned
    // false immediately instead of blocking forever as documented. Passing a null timeval
    // pointer to select() is the correct native equivalent of an infinite wait on both POSIX
    // and Windows (Winsock2's select() honors a null timeout the same way).
    timeval tv{};
    timeval* tvPtr = &tv;
    if (microSeconds == -1) {
        tvPtr = nullptr;
    } else {
        tv.tv_sec = static_cast<long>(microSeconds / 1000000);
        tv.tv_usec = static_cast<long>(microSeconds % 1000000);
    }

    int rc = 0;
    if (mode == SelectMode::SelectRead) {
        rc = ::select(toFd(fd_) + 1, &set, nullptr, nullptr, tvPtr);
    } else if (mode == SelectMode::SelectWrite) {
        rc = ::select(toFd(fd_) + 1, nullptr, &set, nullptr, tvPtr);
    } else {
        rc = ::select(toFd(fd_) + 1, nullptr, nullptr, &set, tvPtr);
    }
    return rc > 0 && FD_ISSET(toSk(fd_), &set);
}

namespace {

    bool mapOption(SocketOptionLevel level, SocketOptionName name, int& nativeLevel, int& nativeName) {
        if (level == SocketOptionLevel::Socket) {
            nativeLevel = SOL_SOCKET;
            switch (name) {
                case SocketOptionName::ReuseAddress: nativeName = SO_REUSEADDR; return true;
                case SocketOptionName::KeepAlive: nativeName = SO_KEEPALIVE; return true;
                case SocketOptionName::Broadcast: nativeName = SO_BROADCAST; return true;
                case SocketOptionName::SendBuffer: nativeName = SO_SNDBUF; return true;
                case SocketOptionName::ReceiveBuffer: nativeName = SO_RCVBUF; return true;
                default: return false;
            }
        }
        if (level == SocketOptionLevel::Tcp && name == SocketOptionName::NoDelay) {
            nativeLevel = IPPROTO_TCP;
            nativeName = TCP_NODELAY;
            return true;
        }
        if (level == SocketOptionLevel::IP) {
            nativeLevel = IPPROTO_IP;
            switch (name) {
                case SocketOptionName::IpTimeToLive: nativeName = IP_TTL; return true;
                case SocketOptionName::MulticastTimeToLive: nativeName = IP_MULTICAST_TTL; return true;
                case SocketOptionName::MulticastLoopback: nativeName = IP_MULTICAST_LOOP; return true;
                default: return false;
            }
        }
        if (level == SocketOptionLevel::IPv6) {
            nativeLevel = IPPROTO_IPV6;
            switch (name) {
                case SocketOptionName::HopLimit: nativeName = IPV6_UNICAST_HOPS; return true;
                case SocketOptionName::IPv6Only: nativeName = IPV6_V6ONLY; return true;
                case SocketOptionName::MulticastLoopback: nativeName = IPV6_MULTICAST_LOOP; return true;
                default: return false;
            }
        }
        return false;
    }

} // namespace

void Socket::SetSocketOption(SocketOptionLevel level, SocketOptionName name, intcs value) {
    int nativeLevel = 0, nativeName = 0;
    if (!mapOption(level, name, nativeLevel, nativeName)) {
        throw SocketException(SocketError::OperationNotSupported,
                               "This (SocketOptionLevel, SocketOptionName) combination is not supported.");
    }
    setOpt(fd_, nativeLevel, nativeName, static_cast<int>(value));
}

void Socket::SetSocketOption(SocketOptionLevel level, SocketOptionName name, bool value) {
    SetSocketOption(level, name, static_cast<intcs>(value ? 1 : 0));
}

void Socket::SetSocketOption(SocketOptionLevel /*level*/, SocketOptionName name, const LingerOption& value) {
    if (name != SocketOptionName::Linger) {
        throw SocketException(SocketError::OperationNotSupported, "Expected SocketOptionName::Linger.");
    }
    setLingerStateProperty(value);
}

intcs Socket::GetSocketOption(SocketOptionLevel level, SocketOptionName name) const {
    int nativeLevel = 0, nativeName = 0;
    if (!mapOption(level, name, nativeLevel, nativeName)) {
        throw SocketException(SocketError::OperationNotSupported,
                               "This (SocketOptionLevel, SocketOptionName) combination is not supported.");
    }
    return getOpt<int>(fd_, nativeLevel, nativeName);
}

bool Socket::getOSSupportsIPv4Property() {
    SockFd s = ::socket(AF_INET, SOCK_DGRAM, 0);
    bool ok = validFd(toFd(s));
    if (ok) closeSk(toFd(s));
    return ok;
}

bool Socket::getOSSupportsIPv6Property() {
    SockFd s = ::socket(AF_INET6, SOCK_DGRAM, 0);
    bool ok = validFd(toFd(s));
    if (ok) closeSk(toFd(s));
    return ok;
}

bool Socket::getOSSupportsUnixDomainSocketsProperty() {
#if defined(AF_UNIX)
    SockFd s = ::socket(AF_UNIX, SOCK_STREAM, 0);
    bool ok = validFd(toFd(s));
    if (ok) closeSk(toFd(s));
    return ok;
#else
    return false;
#endif
}

#endif // !__EMSCRIPTEN__

System::Threading::Tasks::TaskT<bool> Socket::ConnectAsync(const System::Net::EndPoint& remoteEP) {
    // Copies via Serialize()/Create() aren't guaranteed round-trippable for every EndPoint
    // subtype in this runtime, so ConnectAsync only supports IPEndPoint (the common case);
    // for other endpoint types, use the synchronous Connect() from a caller-managed thread.
    const auto* ip = dynamic_cast<const System::Net::IPEndPoint*>(&remoteEP);
    if (ip == nullptr) {
        throw System::ArgumentException("ConnectAsync only supports IPEndPoint in this runtime.", "remoteEP");
    }
    System::Net::IPEndPoint copy(*ip);
    return System::Threading::Tasks::TaskT<bool>([this, copy]() {
        Connect(copy);
        return true;
    });
}

System::Threading::Tasks::TaskT<std::shared_ptr<Socket>> Socket::AcceptAsync() {
    return System::Threading::Tasks::TaskT<std::shared_ptr<Socket>>([this]() { return Accept(); });
}

System::Threading::Tasks::TaskT<intcs> Socket::SendAsync(std::vector<bytecs> buffer, SocketFlags flags) {
    return System::Threading::Tasks::TaskT<intcs>([this, buffer = std::move(buffer), flags]() {
        return Send(buffer, flags);
    });
}

System::Threading::Tasks::TaskT<intcs> Socket::ReceiveAsync(std::shared_ptr<std::vector<bytecs>> buffer,
                                                              SocketFlags flags) {
    return System::Threading::Tasks::TaskT<intcs>([this, buffer, flags]() { return Receive(*buffer, flags); });
}

} // namespace System::Net::Sockets
