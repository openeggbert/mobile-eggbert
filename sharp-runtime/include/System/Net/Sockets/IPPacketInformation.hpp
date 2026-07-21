// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <cstdint>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Net/IPAddress.hpp"

namespace System::Net::Sockets {

    /**
     * @brief Provides the interface and IP address associated with a packet received via
     * Socket::ReceiveMessageFrom().
     *
     * C++ counterpart of .NET System.Net.Sockets.IPPacketInformation.
     */
    class IPPacketInformation {
        System::Net::IPAddress address_;
        SharpRuntime::intcs networkInterface_ = 0;

    public:
        IPPacketInformation() = default;

        IPPacketInformation(System::Net::IPAddress address, SharpRuntime::intcs networkInterface)
            : address_(std::move(address)), networkInterface_(networkInterface) {}

        /** @return The IP address of the received packet. */
        [[nodiscard]] System::Net::IPAddress getAddressProperty() const { return address_; }

        /** @return The network interface index the packet was received on. */
        [[nodiscard]] SharpRuntime::intcs getInterfaceProperty() const { return networkInterface_; }

        [[nodiscard]] bool Equals(const IPPacketInformation& other) const {
            return networkInterface_ == other.networkInterface_ && address_ == other.address_;
        }
        bool operator==(const IPPacketInformation& o) const { return Equals(o); }
        bool operator!=(const IPPacketInformation& o) const { return !Equals(o); }

        /**
         * @brief Returns a hash code combining the interface index and address.
         * C++ counterpart of .NET IPPacketInformation.GetHashCode() -- matches its exact formula:
         * unchecked(interface.GetHashCode() * 0xA5555529) + address.GetHashCode().
         */
        [[nodiscard]] SharpRuntime::intcs GetHashCode() const {
            return static_cast<SharpRuntime::intcs>(
                static_cast<uint32_t>(networkInterface_) * static_cast<uint32_t>(0xA5555529)
                + static_cast<uint32_t>(address_.GetHashCode()));
        }
    };

} // namespace System::Net::Sockets
