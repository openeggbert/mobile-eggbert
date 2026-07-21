// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <optional>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Net/IPAddress.hpp"
#include "System/Net/NetworkInformation/IPStatus.hpp"
#include "System/Net/NetworkInformation/PingOptions.hpp"

namespace System::Net::NetworkInformation {

    /**
     * @brief Provides information about the status and data resulting from a Ping.Send operation.
     *
     * C++ counterpart of .NET System.Net.NetworkInformation.PingReply.
     */
    class PingReply {
        System::Net::IPAddress address_;
        std::optional<PingOptions> options_;
        IPStatus status_ = IPStatus::Unknown;
        SharpRuntime::longcs roundtripTime_ = 0;
        std::vector<SharpRuntime::bytecs> buffer_;

    public:
        /** @brief Default-constructs an empty reply (status Unknown) — needed so PingReply can be TaskT<PingReply>'s result type. */
        PingReply() = default;

        PingReply(System::Net::IPAddress address, std::optional<PingOptions> options, IPStatus status,
                   SharpRuntime::longcs roundtripTime, std::vector<SharpRuntime::bytecs> buffer)
            : address_(std::move(address)), options_(std::move(options)), status_(status),
              roundtripTime_(roundtripTime), buffer_(std::move(buffer)) {}

        /** @return The status of the ICMP echo reply. */
        [[nodiscard]] IPStatus getStatusProperty() const { return status_; }

        /** @return The address of the host that sent the reply. */
        [[nodiscard]] System::Net::IPAddress getAddressProperty() const { return address_; }

        /** @return The round-trip time in milliseconds. Zero unless getStatusProperty() is Success. */
        [[nodiscard]] SharpRuntime::longcs getRoundtripTimeProperty() const { return roundtripTime_; }

        /** @return The options used to send the request, or empty if none were specified. */
        [[nodiscard]] const std::optional<PingOptions>& getOptionsProperty() const { return options_; }

        /** @return The buffer of data received in the reply. */
        [[nodiscard]] const std::vector<SharpRuntime::bytecs>& getBufferProperty() const { return buffer_; }
    };

} // namespace System::Net::NetworkInformation
