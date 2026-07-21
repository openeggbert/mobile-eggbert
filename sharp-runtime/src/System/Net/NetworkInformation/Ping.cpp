// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Net/NetworkInformation/Ping.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Net/Dns.hpp"
#include "System/Net/NetworkInformation/NetworkInformationException.hpp"
#include "System/Net/NetworkInformation/PingException.hpp"
#include "System/PlatformNotSupportedException.hpp"

#if defined(_WIN32)
// No Windows PAL implemented yet.
#elif defined(__EMSCRIPTEN__)
// No raw/ping-socket support available under Emscripten.
#else
#define SHARP_RUNTIME_PING_POSIX 1
#include <arpa/inet.h>
#include <atomic>
#include <cerrno>
#include <chrono>
#include <cstring>
#include <netinet/icmp6.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

namespace System::Net::NetworkInformation {

std::vector<SharpRuntime::bytecs> Ping::defaultSendBuffer() {
    std::vector<SharpRuntime::bytecs> buffer(DefaultSendBufferSize);
    for (SharpRuntime::intcs i = 0; i < DefaultSendBufferSize; ++i) {
        buffer[static_cast<size_t>(i)] = static_cast<SharpRuntime::bytecs>('a' + (i % 23));
    }
    return buffer;
}

void Ping::checkArgs(SharpRuntime::intcs timeout, const std::vector<SharpRuntime::bytecs>& buffer) {
    if (buffer.size() > static_cast<size_t>(MaxBufferSize)) {
        throw System::ArgumentException("The data buffer is too long. It must be 65,500 bytes or less.", "buffer");
    }
    System::ArgumentOutOfRangeException::ThrowIfNegative(timeout, "timeout");
}

void Ping::checkArgs(const System::Net::IPAddress& address, SharpRuntime::intcs timeout,
                      const std::vector<SharpRuntime::bytecs>& buffer) {
    checkArgs(timeout, buffer);
    if (address == System::Net::IPAddress::Any || address == System::Net::IPAddress::IPv6Any) {
        throw System::ArgumentException("An invalid IP address was specified.", "address");
    }
}

#if defined(SHARP_RUNTIME_PING_POSIX)
namespace {

    uint16_t internetChecksum(const void* data, size_t len) {
        const auto* p = static_cast<const uint8_t*>(data);
        uint32_t sum = 0;
        while (len > 1) {
            sum += (static_cast<uint32_t>(p[0]) << 8) | p[1];
            p += 2;
            len -= 2;
        }
        if (len == 1) {
            sum += static_cast<uint32_t>(p[0]) << 8;
        }
        while (sum >> 16) {
            sum = (sum & 0xFFFF) + (sum >> 16);
        }
        return static_cast<uint16_t>(~sum);
    }

    IPStatus mapIcmpV4Status(uint8_t type, uint8_t code) {
        switch (type) {
            case ICMP_ECHOREPLY:
                return IPStatus::Success;
            case ICMP_DEST_UNREACH:
                switch (code) {
                    case ICMP_NET_UNREACH:
                        return IPStatus::DestinationNetworkUnreachable;
                    case ICMP_HOST_UNREACH:
                        return IPStatus::DestinationHostUnreachable;
                    case ICMP_PROT_UNREACH:
                        return IPStatus::DestinationProtocolUnreachable;
                    case ICMP_PORT_UNREACH:
                        return IPStatus::DestinationPortUnreachable;
                    default:
                        return IPStatus::DestinationUnreachable;
                }
            case ICMP_TIME_EXCEEDED:
                return IPStatus::TtlExpired;
            default:
                return IPStatus::Unknown;
        }
    }

    IPStatus mapIcmpV6Status(uint8_t type) {
        switch (type) {
            case ICMP6_ECHO_REPLY:
                return IPStatus::Success;
            case ICMP6_DST_UNREACH:
                return IPStatus::DestinationUnreachable;
            case ICMP6_TIME_EXCEEDED:
                return IPStatus::TtlExpired;
            default:
                return IPStatus::Unknown;
        }
    }

    int createIcmpSocket(bool isIPv6) {
        int family = isIPv6 ? static_cast<int>(AF_INET6) : static_cast<int>(AF_INET);
        int protocol = isIPv6 ? static_cast<int>(IPPROTO_ICMPV6) : static_cast<int>(IPPROTO_ICMP);
        int fd = ::socket(family, SOCK_DGRAM, protocol);
        if (fd < 0) {
            throw NetworkInformationException();
        }
        return fd;
    }

    void applyOptions(int fd, bool isIPv6, const PingOptions* options) {
        if (options == nullptr) {
            return;
        }
        int ttl = static_cast<int>(options->getTtlProperty());
        if (isIPv6) {
            ::setsockopt(fd, IPPROTO_IPV6, IPV6_UNICAST_HOPS, &ttl, sizeof(ttl));
        } else {
            ::setsockopt(fd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));
#if defined(IP_MTU_DISCOVER)
            int mode = options->getDontFragmentProperty() ? IP_PMTUDISC_DO : IP_PMTUDISC_WANT;
            ::setsockopt(fd, IPPROTO_IP, IP_MTU_DISCOVER, &mode, sizeof(mode));
#endif
        }
    }

} // namespace

PingReply Ping::sendPingCore(const System::Net::IPAddress& address, const std::vector<SharpRuntime::bytecs>& buffer,
                               SharpRuntime::intcs timeout, const PingOptions* options) {
    bool isIPv6 = address.getIsIPv6Property();
    int fd = createIcmpSocket(isIPv6);
    applyOptions(fd, isIPv6, options);

    timeval tv{};
    tv.tv_sec = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;
    ::setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    static std::atomic<uint16_t> sequenceCounter{0};
    uint16_t identifier = static_cast<uint16_t>(::getpid() & 0xFFFF);
    uint16_t sequence = ++sequenceCounter;

    std::vector<uint8_t> packet;
    if (isIPv6) {
        packet.resize(sizeof(icmp6_hdr) + buffer.size());
        // Build the header in a local, properly-typed object rather than reinterpret_cast'ing
        // packet.data() (a uint8_t*) to icmp6_hdr* and memset'ing through it -- GCC 14's
        // -Warray-bounds (enabled by -Werror in Release) cannot prove the vector's dynamic
        // storage is large enough through that cast and flags it as an out-of-bounds write.
        icmp6_hdr hdr{};
        hdr.icmp6_type = ICMP6_ECHO_REQUEST;
        hdr.icmp6_code = 0;
        hdr.icmp6_id = htons(identifier);
        hdr.icmp6_seq = htons(sequence);
        std::memcpy(packet.data(), &hdr, sizeof(hdr));
        if (!buffer.empty()) {
            std::memcpy(packet.data() + sizeof(icmp6_hdr), buffer.data(), buffer.size());
        }
    } else {
        packet.resize(sizeof(icmphdr) + buffer.size());
        // Same rationale as the icmp6_hdr branch above.
        icmphdr hdr{};
        hdr.type = ICMP_ECHO;
        hdr.code = 0;
        hdr.checksum = 0;
        hdr.un.echo.id = htons(identifier);
        hdr.un.echo.sequence = htons(sequence);
        std::memcpy(packet.data(), &hdr, sizeof(hdr));
        if (!buffer.empty()) {
            std::memcpy(packet.data() + sizeof(icmphdr), buffer.data(), buffer.size());
        }
        // The checksum covers the whole packet (header + payload), so it can only be computed
        // once the payload has been copied in; patch it into both the local header and the
        // packet buffer afterward.
        hdr.checksum = htons(internetChecksum(packet.data(), packet.size()));
        std::memcpy(packet.data(), &hdr, sizeof(hdr));
    }

    sockaddr_storage dest{};
    socklen_t destLen;
    if (isIPv6) {
        auto* dst6 = reinterpret_cast<sockaddr_in6*>(&dest);
        dst6->sin6_family = AF_INET6;
        auto bytes = address.GetAddressBytes();
        std::memcpy(&dst6->sin6_addr, bytes.data(), bytes.size());
        destLen = sizeof(sockaddr_in6);
    } else {
        auto* dst4 = reinterpret_cast<sockaddr_in*>(&dest);
        dst4->sin_family = AF_INET;
        dst4->sin_addr.s_addr = htonl(address.getAddressProperty());
        destLen = sizeof(sockaddr_in);
    }

    auto start = std::chrono::steady_clock::now();
    ssize_t sent = ::sendto(fd, packet.data(), packet.size(), 0, reinterpret_cast<sockaddr*>(&dest), destLen);
    if (sent < 0) {
        int err = errno;
        ::close(fd);
        throw NetworkInformationException(err);
    }

    std::vector<uint8_t> recvBuf(65535);
    ssize_t received = ::recv(fd, recvBuf.data(), recvBuf.size(), 0);
    auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
    ::close(fd);

    if (received < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return PingReply(address, options != nullptr ? std::make_optional(*options) : std::nullopt,
                              IPStatus::TimedOut, 0, {});
        }
        throw NetworkInformationException(errno);
    }

    IPStatus status = IPStatus::Unknown;
    std::vector<SharpRuntime::bytecs> replyBuffer;
    if (isIPv6) {
        if (static_cast<size_t>(received) >= sizeof(icmp6_hdr)) {
            const auto* hdr = reinterpret_cast<const icmp6_hdr*>(recvBuf.data());
            status = mapIcmpV6Status(hdr->icmp6_type);
            if (static_cast<size_t>(received) > sizeof(icmp6_hdr)) {
                replyBuffer.assign(recvBuf.begin() + sizeof(icmp6_hdr), recvBuf.begin() + received);
            }
        }
    } else {
        if (static_cast<size_t>(received) >= sizeof(icmphdr)) {
            const auto* hdr = reinterpret_cast<const icmphdr*>(recvBuf.data());
            status = mapIcmpV4Status(hdr->type, hdr->code);
            if (static_cast<size_t>(received) > sizeof(icmphdr)) {
                replyBuffer.assign(recvBuf.begin() + sizeof(icmphdr), recvBuf.begin() + received);
            }
        }
    }

    return PingReply(address, options != nullptr ? std::make_optional(*options) : std::nullopt, status,
                      static_cast<SharpRuntime::longcs>(status == IPStatus::Success ? elapsedMs : 0), replyBuffer);
}

#else

PingReply Ping::sendPingCore(const System::Net::IPAddress&, const std::vector<SharpRuntime::bytecs>&,
                               SharpRuntime::intcs, const PingOptions*) {
    throw System::PlatformNotSupportedException("Ping is only implemented on POSIX platforms in this runtime.");
}

#endif

namespace {

    PingReply sendWithExceptionWrapping(const System::Net::IPAddress& address,
                                          const std::vector<SharpRuntime::bytecs>& buffer, SharpRuntime::intcs timeout,
                                          const PingOptions* options,
                                          PingReply (*core)(const System::Net::IPAddress&,
                                                             const std::vector<SharpRuntime::bytecs>&,
                                                             SharpRuntime::intcs, const PingOptions*)) {
        try {
            return core(address, buffer, timeout, options);
        } catch (const System::PlatformNotSupportedException&) {
            throw;
        } catch (const std::exception& e) {
            throw PingException("An exception occurred while sending or receiving the ICMP message.",
                                 std::make_exception_ptr(e));
        }
    }

} // namespace

PingReply Ping::Send(const std::string& hostNameOrAddress) {
    return Send(hostNameOrAddress, DefaultTimeout, defaultSendBuffer());
}

PingReply Ping::Send(const std::string& hostNameOrAddress, SharpRuntime::intcs timeout) {
    return Send(hostNameOrAddress, timeout, defaultSendBuffer());
}

PingReply Ping::Send(const System::Net::IPAddress& address) {
    return Send(address, DefaultTimeout, defaultSendBuffer());
}

PingReply Ping::Send(const System::Net::IPAddress& address, SharpRuntime::intcs timeout) {
    return Send(address, timeout, defaultSendBuffer());
}

PingReply Ping::Send(const std::string& hostNameOrAddress, SharpRuntime::intcs timeout,
                      const std::vector<SharpRuntime::bytecs>& buffer) {
    return Send(hostNameOrAddress, timeout, buffer, PingOptions());
}

PingReply Ping::Send(const System::Net::IPAddress& address, SharpRuntime::intcs timeout,
                      const std::vector<SharpRuntime::bytecs>& buffer) {
    checkArgs(address, timeout, buffer);
    return sendWithExceptionWrapping(address, buffer, timeout, nullptr, &Ping::sendPingCore);
}

PingReply Ping::Send(const std::string& hostNameOrAddress, SharpRuntime::intcs timeout,
                      const std::vector<SharpRuntime::bytecs>& buffer, const PingOptions& options) {
    System::ArgumentException::ThrowIfNullOrEmpty(hostNameOrAddress, "hostNameOrAddress");

    System::Net::IPAddress parsed;
    if (System::Net::IPAddress::TryParse(hostNameOrAddress, parsed)) {
        return Send(parsed, timeout, buffer, options);
    }

    checkArgs(timeout, buffer);
    auto addresses = System::Net::Dns::GetHostAddresses(hostNameOrAddress);
    if (addresses.empty()) {
        throw PingException("Could not resolve host name or address.");
    }
    return sendWithExceptionWrapping(addresses[0], buffer, timeout, &options, &Ping::sendPingCore);
}

PingReply Ping::Send(const System::Net::IPAddress& address, SharpRuntime::intcs timeout,
                      const std::vector<SharpRuntime::bytecs>& buffer, const PingOptions& options) {
    checkArgs(address, timeout, buffer);
    return sendWithExceptionWrapping(address, buffer, timeout, &options, &Ping::sendPingCore);
}

System::Threading::Tasks::TaskT<PingReply> Ping::SendPingAsync(const System::Net::IPAddress& address) {
    return SendPingAsync(address, DefaultTimeout, defaultSendBuffer());
}

System::Threading::Tasks::TaskT<PingReply> Ping::SendPingAsync(const std::string& hostNameOrAddress) {
    return SendPingAsync(hostNameOrAddress, DefaultTimeout, defaultSendBuffer());
}

System::Threading::Tasks::TaskT<PingReply> Ping::SendPingAsync(const System::Net::IPAddress& address,
                                                                 SharpRuntime::intcs timeout) {
    return SendPingAsync(address, timeout, defaultSendBuffer());
}

System::Threading::Tasks::TaskT<PingReply> Ping::SendPingAsync(const std::string& hostNameOrAddress,
                                                                 SharpRuntime::intcs timeout) {
    return SendPingAsync(hostNameOrAddress, timeout, defaultSendBuffer());
}

System::Threading::Tasks::TaskT<PingReply> Ping::SendPingAsync(const System::Net::IPAddress& address,
                                                                 SharpRuntime::intcs timeout,
                                                                 const std::vector<SharpRuntime::bytecs>& buffer) {
    return SendPingAsync(address, timeout, buffer, PingOptions());
}

System::Threading::Tasks::TaskT<PingReply> Ping::SendPingAsync(const std::string& hostNameOrAddress,
                                                                 SharpRuntime::intcs timeout,
                                                                 const std::vector<SharpRuntime::bytecs>& buffer) {
    return SendPingAsync(hostNameOrAddress, timeout, buffer, PingOptions());
}

System::Threading::Tasks::TaskT<PingReply> Ping::SendPingAsync(const System::Net::IPAddress& address,
                                                                 SharpRuntime::intcs timeout,
                                                                 const std::vector<SharpRuntime::bytecs>& buffer,
                                                                 const PingOptions& options) {
    return System::Threading::Tasks::TaskT<PingReply>(
        [this, address, timeout, buffer, options]() { return Send(address, timeout, buffer, options); });
}

System::Threading::Tasks::TaskT<PingReply> Ping::SendPingAsync(const std::string& hostNameOrAddress,
                                                                 SharpRuntime::intcs timeout,
                                                                 const std::vector<SharpRuntime::bytecs>& buffer,
                                                                 const PingOptions& options) {
    return System::Threading::Tasks::TaskT<PingReply>(
        [this, hostNameOrAddress, timeout, buffer, options]() { return Send(hostNameOrAddress, timeout, buffer, options); });
}

} // namespace System::Net::NetworkInformation
