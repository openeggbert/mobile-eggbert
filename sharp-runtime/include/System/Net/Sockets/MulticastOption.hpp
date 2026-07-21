// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Net/IPAddress.hpp"

namespace System::Net::Sockets {

    /**
     * @brief Contains option values for IPv4 multicast packets.
     *
     * C++ counterpart of .NET System.Net.Sockets.MulticastOption.
     */
    class MulticastOption {
        System::Net::IPAddress group_;
        System::Net::IPAddress localAddress_ = System::Net::IPAddress::Any;
        SharpRuntime::intcs interfaceIndex_ = 0;

    public:
        /** @brief Constructs for @p group with a specific local interface address. */
        MulticastOption(System::Net::IPAddress group, System::Net::IPAddress mcint)
            : group_(std::move(group)), localAddress_(std::move(mcint)) {}

        /** @brief Constructs for @p group with a specific local interface index. */
        MulticastOption(System::Net::IPAddress group, SharpRuntime::intcs interfaceIndex) : group_(std::move(group)) {
            System::ArgumentOutOfRangeException::ThrowIfNegative(interfaceIndex, "interfaceIndex");
            System::ArgumentOutOfRangeException::ThrowIfGreaterThan(interfaceIndex, 0x00FFFFFF, "interfaceIndex");
            interfaceIndex_ = interfaceIndex;
        }

        /** @brief Constructs for @p group, using IPAddress::Any as the local interface. */
        explicit MulticastOption(System::Net::IPAddress group) : group_(std::move(group)) {}

        /** @return The multicast group address. */
        [[nodiscard]] System::Net::IPAddress getGroupProperty() const { return group_; }
        /** @brief Sets the multicast group address. */
        void setGroupProperty(System::Net::IPAddress value) { group_ = std::move(value); }

        /** @return The local interface address, if set via the address-based constructor/setter. */
        [[nodiscard]] System::Net::IPAddress getLocalAddressProperty() const { return localAddress_; }
        /** @brief Sets the local interface address (clears any interface index previously set). */
        void setLocalAddressProperty(System::Net::IPAddress value) {
            interfaceIndex_ = 0;
            localAddress_ = std::move(value);
        }

        /** @return The local interface index, if set via the index-based constructor/setter. */
        [[nodiscard]] SharpRuntime::intcs getInterfaceIndexProperty() const { return interfaceIndex_; }
        /** @brief Sets the local interface index (clears any local address previously set). */
        void setInterfaceIndexProperty(SharpRuntime::intcs value) {
            System::ArgumentOutOfRangeException::ThrowIfNegative(value, "value");
            System::ArgumentOutOfRangeException::ThrowIfGreaterThan(value, 0x00FFFFFF, "value");
            interfaceIndex_ = value;
        }
    };

    /**
     * @brief Contains option values for joining an IPv6 multicast group.
     *
     * C++ counterpart of .NET System.Net.Sockets.IPv6MulticastOption.
     */
    class IPv6MulticastOption {
        System::Net::IPAddress group_;
        SharpRuntime::longcs interfaceIndex_ = 0;

    public:
        /** @brief Constructs for @p group with a specific local interface index. */
        IPv6MulticastOption(System::Net::IPAddress group, SharpRuntime::longcs ifIndex) : group_(std::move(group)) {
            System::ArgumentOutOfRangeException::ThrowIfNegative(ifIndex, "ifIndex");
            System::ArgumentOutOfRangeException::ThrowIfGreaterThan(ifIndex, static_cast<SharpRuntime::longcs>(0xFFFFFFFFLL),
                                                                     "ifIndex");
            interfaceIndex_ = ifIndex;
        }

        /** @brief Constructs for @p group with interface index 0. */
        explicit IPv6MulticastOption(System::Net::IPAddress group) : group_(std::move(group)) {}

        /** @return The multicast group address. */
        [[nodiscard]] System::Net::IPAddress getGroupProperty() const { return group_; }
        /** @brief Sets the multicast group address. */
        void setGroupProperty(System::Net::IPAddress value) { group_ = std::move(value); }

        /** @return The local interface index. */
        [[nodiscard]] SharpRuntime::longcs getInterfaceIndexProperty() const { return interfaceIndex_; }
        /** @brief Sets the local interface index. */
        void setInterfaceIndexProperty(SharpRuntime::longcs value) {
            System::ArgumentOutOfRangeException::ThrowIfNegative(value, "value");
            System::ArgumentOutOfRangeException::ThrowIfGreaterThan(value, static_cast<SharpRuntime::longcs>(0xFFFFFFFFLL),
                                                                     "value");
            interfaceIndex_ = value;
        }
    };

} // namespace System::Net::Sockets
