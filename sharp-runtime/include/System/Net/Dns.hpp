// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include <vector>
#include "System/Net/IPAddress.hpp"
#include "System/Net/IPHostEntry.hpp"
#include "System/Net/Sockets/AddressFamily.hpp"

namespace System::Net {

    /**
     * @brief Provides simple domain name resolution functionality.
     *
     * Partial C++ counterpart of .NET System.Net.Dns.
     *
     * @note Status: Partial. Only the synchronous, non-obsolete surface is ported
     * (GetHostName, GetHostAddresses, GetHostEntry). The async Task-returning overloads,
     * the IAsyncResult-based Begin/End methods, and the obsolete GetHostByName/Resolve/
     * GetHostByAddress legacy methods are not ported, since this runtime's Task
     * infrastructure doesn't route real async I/O (see NEXT.md, System.Threading.Tasks
     * limitations), so those overloads would just be synchronous work wrapped in a Task
     * for no benefit.
     * @note Resolution is restricted to IPv4: the getaddrinfo/GetAddrInfoW hints
     * passed by this implementation always request AF_INET, regardless of the
     * @p family argument (System::Net::IPAddress itself supports IPv6 fine).
     * Requesting AddressFamily::InterNetworkV6 explicitly returns an empty result
     * rather than throwing or actually resolving IPv6 addresses.
     * @note POSIX (getaddrinfo/getnameinfo/gethostname) and Windows (Winsock2) are both
     * implemented; Emscripten throws PlatformNotSupportedException.
     */
    class Dns {
    public:
        Dns() = delete;

        /** @return The host name of the local machine. */
        static std::string GetHostName();

        /** Resolves a host name or IP address string to its IPv4 addresses. */
        static std::vector<IPAddress> GetHostAddresses(const std::string& hostNameOrAddress);

        /** Resolves a host name or IP address string to addresses of the given family. */
        static std::vector<IPAddress> GetHostAddresses(const std::string& hostNameOrAddress, System::Net::Sockets::AddressFamily family);

        /** Resolves a host name or IP address string to an IPHostEntry. */
        static IPHostEntry GetHostEntry(const std::string& hostNameOrAddress);

        /** Resolves a host name or IP address string to an IPHostEntry, restricted to the given family. */
        static IPHostEntry GetHostEntry(const std::string& hostNameOrAddress, System::Net::Sockets::AddressFamily family);

        /** Resolves an IPAddress to an IPHostEntry via reverse then forward DNS lookup. */
        static IPHostEntry GetHostEntry(const IPAddress& address);
    };

} // namespace System::Net
