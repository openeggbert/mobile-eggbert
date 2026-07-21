// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <optional>
#include <string>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Net/IPAddress.hpp"
#include "System/Net/NetworkInformation/PingOptions.hpp"
#include "System/Net/NetworkInformation/PingReply.hpp"
#include "System/Threading/Tasks/Task.hpp"

namespace System::Net::NetworkInformation {

    /**
     * @brief Allows applications to determine whether a remote computer is reachable, using ICMP.
     *
     * C++ counterpart of .NET System.Net.NetworkInformation.Ping.
     *
     * @note Reduced scope: `SendAsync`/`SendAsyncCancel`/the `PingCompleted` event (the legacy
     * event-based async pattern) are not reproduced — `PingCompletedEventArgs`/
     * `PingCompletedEventHandler` are already marked out of scope in `plan.sqlite3` (legacy DB
     * status, predating this workflow). `SendPingAsync` (the modern Task-based API) is implemented
     * instead, matching the pattern used for `System.Net.Http.Json`'s `*Async` extension methods.
     * @note Implemented via an unprivileged `SOCK_DGRAM`+`IPPROTO_ICMP`/`IPPROTO_ICMPV6` "ping
     * socket" (Linux `net.ipv4.ping_group_range`), not a raw `SOCK_RAW` socket — this avoids
     * requiring elevated privileges, and was confirmed to work end-to-end (real ICMP echo
     * round-trip to loopback) in this runtime's sandboxed test environment before implementing.
     * POSIX/Linux-only: throws `System::PlatformNotSupportedException` elsewhere.
     */
    class Ping {
        static constexpr SharpRuntime::intcs DefaultSendBufferSize = 32;
        static constexpr SharpRuntime::intcs DefaultTimeout = 5000;
        static constexpr SharpRuntime::intcs MaxBufferSize = 65500;

        static std::vector<SharpRuntime::bytecs> defaultSendBuffer();
        static void checkArgs(SharpRuntime::intcs timeout, const std::vector<SharpRuntime::bytecs>& buffer);
        static void checkArgs(const System::Net::IPAddress& address, SharpRuntime::intcs timeout,
                               const std::vector<SharpRuntime::bytecs>& buffer);

        static PingReply sendPingCore(const System::Net::IPAddress& address,
                                       const std::vector<SharpRuntime::bytecs>& buffer, SharpRuntime::intcs timeout,
                                       const PingOptions* options);

    public:
        Ping() = default;

        /** @brief Sends an ICMP echo request to @p hostNameOrAddress and waits for the reply. */
        [[nodiscard]] PingReply Send(const std::string& hostNameOrAddress);
        /** @brief As above, with an explicit @p timeout in milliseconds. */
        [[nodiscard]] PingReply Send(const std::string& hostNameOrAddress, SharpRuntime::intcs timeout);
        /** @brief Sends an ICMP echo request to @p address and waits for the reply. */
        [[nodiscard]] PingReply Send(const System::Net::IPAddress& address);
        /** @brief As above, with an explicit @p timeout in milliseconds. */
        [[nodiscard]] PingReply Send(const System::Net::IPAddress& address, SharpRuntime::intcs timeout);
        /** @brief As above, with a custom data @p buffer (echoed back in the reply). */
        [[nodiscard]] PingReply Send(const std::string& hostNameOrAddress, SharpRuntime::intcs timeout,
                                      const std::vector<SharpRuntime::bytecs>& buffer);
        /** @brief As above, with a custom data @p buffer (echoed back in the reply). */
        [[nodiscard]] PingReply Send(const System::Net::IPAddress& address, SharpRuntime::intcs timeout,
                                      const std::vector<SharpRuntime::bytecs>& buffer);
        /** @brief As above, with @p options controlling TTL/fragmentation. */
        [[nodiscard]] PingReply Send(const std::string& hostNameOrAddress, SharpRuntime::intcs timeout,
                                      const std::vector<SharpRuntime::bytecs>& buffer, const PingOptions& options);
        /** @brief As above, with @p options controlling TTL/fragmentation. */
        [[nodiscard]] PingReply Send(const System::Net::IPAddress& address, SharpRuntime::intcs timeout,
                                      const std::vector<SharpRuntime::bytecs>& buffer, const PingOptions& options);

        /** @brief Asynchronous counterpart of Send(address). */
        [[nodiscard]] System::Threading::Tasks::TaskT<PingReply> SendPingAsync(const System::Net::IPAddress& address);
        /** @brief Asynchronous counterpart of Send(hostNameOrAddress). */
        [[nodiscard]] System::Threading::Tasks::TaskT<PingReply> SendPingAsync(const std::string& hostNameOrAddress);
        /** @brief Asynchronous counterpart of Send(address, timeout). */
        [[nodiscard]] System::Threading::Tasks::TaskT<PingReply> SendPingAsync(const System::Net::IPAddress& address,
                                                                                SharpRuntime::intcs timeout);
        /** @brief Asynchronous counterpart of Send(hostNameOrAddress, timeout). */
        [[nodiscard]] System::Threading::Tasks::TaskT<PingReply> SendPingAsync(const std::string& hostNameOrAddress,
                                                                                SharpRuntime::intcs timeout);
        /** @brief Asynchronous counterpart of Send(address, timeout, buffer). */
        [[nodiscard]] System::Threading::Tasks::TaskT<PingReply>
        SendPingAsync(const System::Net::IPAddress& address, SharpRuntime::intcs timeout,
                      const std::vector<SharpRuntime::bytecs>& buffer);
        /** @brief Asynchronous counterpart of Send(hostNameOrAddress, timeout, buffer). */
        [[nodiscard]] System::Threading::Tasks::TaskT<PingReply>
        SendPingAsync(const std::string& hostNameOrAddress, SharpRuntime::intcs timeout,
                      const std::vector<SharpRuntime::bytecs>& buffer);
        /** @brief Asynchronous counterpart of Send(address, timeout, buffer, options). */
        [[nodiscard]] System::Threading::Tasks::TaskT<PingReply>
        SendPingAsync(const System::Net::IPAddress& address, SharpRuntime::intcs timeout,
                      const std::vector<SharpRuntime::bytecs>& buffer, const PingOptions& options);
        /** @brief Asynchronous counterpart of Send(hostNameOrAddress, timeout, buffer, options). */
        [[nodiscard]] System::Threading::Tasks::TaskT<PingReply>
        SendPingAsync(const std::string& hostNameOrAddress, SharpRuntime::intcs timeout,
                      const std::vector<SharpRuntime::bytecs>& buffer, const PingOptions& options);
    };

} // namespace System::Net::NetworkInformation
