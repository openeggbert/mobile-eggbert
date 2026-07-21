// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/Net/EndPoint.hpp"
#include "System/Net/IPAddress.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Net {

    using SharpRuntime::intcs;
    using SharpRuntime::longcs;

    /**
     * Represents a network endpoint as an IP address and a port number.
     *
     * C++ counterpart of .NET System.Net.IPEndPoint.
     *
     * @note The Span/UTF8-based Parse/TryParse/TryFormat overloads from .NET's
     * ISpanParsable/IUtf8SpanParsable surface are not ported; only std::string
     * overloads are, since this runtime has no Span<T>/UTF8-string idiom.
     */
    class IPEndPoint : public EndPoint {
        IPAddress address_;
        intcs     port_ = 0;

        static void validatePort(intcs port);

    public:
        /** Minimum valid port number. */
        static constexpr intcs MinPort = 0;
        /** Maximum valid port number. */
        static constexpr intcs MaxPort = 65535;

        /** Default constructor — 0.0.0.0:0. */
        IPEndPoint() = default;

        /** Constructs an endpoint from an IPAddress and port. */
        IPEndPoint(const IPAddress& address, intcs port);

        /** Constructs an IPv4 endpoint from a raw 32-bit host-order address and port. */
        IPEndPoint(uint32_t address, intcs port);

        /** Constructs an IPv4 endpoint from a 64-bit address value and port (matches .NET's long-based constructor). */
        IPEndPoint(longcs address, intcs port);

        /** @return The address family of the underlying IP address. */
        [[nodiscard]] System::Net::Sockets::AddressFamily getAddressFamilyProperty() const override {
            return address_.getAddressFamilyProperty();
        }

        /** @return The IP address component. */
        [[nodiscard]] const IPAddress& getAddressProperty() const { return address_; }
        /** Sets the IP address component. */
        void setAddressProperty(const IPAddress& a) { address_ = a; }

        /** @return The port number. */
        [[nodiscard]] intcs getPortProperty() const { return port_; }
        /**
         * Sets the port number.
         * @throws System::ArgumentOutOfRangeException if @p p is outside [MinPort, MaxPort].
         */
        void setPortProperty(intcs p);

        /** @return "address:port" for IPv4, "[address]:port" for IPv6. */
        [[nodiscard]] std::string ToString() const;

        /** Serializes this endpoint into a SocketAddress. */
        [[nodiscard]] SocketAddress Serialize() const override;

        /** Creates an IPEndPoint from a SocketAddress produced by Serialize(). */
        [[nodiscard]] std::shared_ptr<EndPoint> Create(const SocketAddress& socketAddress) const override;

        /** Equality comparison: same address and port. */
        [[nodiscard]] bool Equals(const IPEndPoint& other) const {
            return address_ == other.address_ && port_ == other.port_;
        }
        /** Equality operator. */
        bool operator==(const IPEndPoint& o) const { return Equals(o); }
        /** Inequality operator. */
        bool operator!=(const IPEndPoint& o) const { return !(*this == o); }

        /** @return A hash code combining the address and port. */
        [[nodiscard]] intcs GetHashCode() const {
            return address_.GetHashCode() ^ port_;
        }

        /**
         * Parses "address:port" (IPv4) or "[address]:port" (IPv6) into an IPEndPoint.
         * @throws System::FormatException if @p s is not a valid endpoint string.
         */
        [[nodiscard]] static IPEndPoint Parse(const std::string& s);

        /** Attempts to parse @p s as an IPEndPoint; returns false without throwing on failure. */
        [[nodiscard]] static bool TryParse(const std::string& s, IPEndPoint& result);
    };

} // namespace System::Net
