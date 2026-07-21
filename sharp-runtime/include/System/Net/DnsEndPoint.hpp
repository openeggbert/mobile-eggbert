// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/Net/EndPoint.hpp"
#include "System/Net/Sockets/AddressFamily.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Net {

    using SharpRuntime::intcs;

    /**
     * @brief Represents a network endpoint as a host name (or string IP address) and a port number.
     *
     * C++ counterpart of .NET System.Net.DnsEndPoint.
     */
    class DnsEndPoint : public EndPoint {
        std::string host_;
        intcs port_ = 0;
        System::Net::Sockets::AddressFamily family_ = System::Net::Sockets::AddressFamily::Unspecified;

    public:
        /** Constructs a DnsEndPoint with an unspecified address family. */
        DnsEndPoint(const std::string& host, intcs port);

        /** Constructs a DnsEndPoint restricted to the given address family. */
        DnsEndPoint(const std::string& host, intcs port, System::Net::Sockets::AddressFamily addressFamily);

        /** @return The host name or IP address string. */
        [[nodiscard]] const std::string& getHostProperty() const { return host_; }

        /** @return The port number. */
        [[nodiscard]] intcs getPortProperty() const { return port_; }

        /** @return The address family to which the endpoint belongs. */
        [[nodiscard]] System::Net::Sockets::AddressFamily getAddressFamilyProperty() const override { return family_; }

        /** @return "{family}/{host}:{port}". */
        [[nodiscard]] std::string ToString() const;

        /** Equality comparison: matches on address family, port, and case-insensitive host. */
        [[nodiscard]] bool Equals(const DnsEndPoint& other) const;
        bool operator==(const DnsEndPoint& other) const { return Equals(other); }
        bool operator!=(const DnsEndPoint& other) const { return !Equals(other); }
    };

} // namespace System::Net
