// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <array>
#include <string>
#include <vector>
#include "System/Net/Sockets/AddressFamily.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Net {

    using SharpRuntime::bytecs;
    using SharpRuntime::intcs;
    using SharpRuntime::longcs;
    using SharpRuntime::shortcs;

    /**
     * Provides an Internet Protocol (IP) address.
     *
     * C++ counterpart of .NET System.Net.IPAddress. Supports both IPv4 and IPv6.
     *
     * @note This runtime has no Span<T>/UTF8-string idiom, so the UTF8 and
     * Span-based Parse/TryParse/TryFormat overloads from .NET's ISpanParsable/
     * IUtf8SpanParsable surface are not ported; only the std::string overloads are.
     * @note IPv6 scope IDs are numeric only — there is no interface-name-to-index
     * resolution (e.g. "%eth0"), since this runtime has no network interface
     * enumeration API.
     */
    class IPAddress {
        bool isIPv6_ = false;
        uint32_t addressOrScopeId_ = 0; ///< IPv4: address in host byte order. IPv6: scope ID.
        std::array<uint16_t, 8> numbers_{}; ///< IPv6 only: each element is one 16-bit group's value.

        static constexpr uint32_t LoopbackMaskHostOrder = 0xFF000000;

        IPAddress(const std::array<uint16_t, 8>& numbers, uint32_t scopeId)
            : isIPv6_(true), addressOrScopeId_(scopeId), numbers_(numbers) {}

    public:
        /** Default constructor — produces 0.0.0.0 (IPv4). */
        IPAddress() = default;

        /** Constructs an IPv4 address from a 32-bit host-byte-order integer. */
        explicit IPAddress(uint32_t address) : addressOrScopeId_(address) {}

        /**
         * Constructs an IPv4 or IPv6 address from raw address bytes (4 bytes = IPv4, 16 bytes = IPv6).
         * @throws System::ArgumentException if @p addressBytes is not 4 or 16 bytes long.
         */
        explicit IPAddress(const std::vector<bytecs>& addressBytes);

        /** Constructs an IPv6 address with the given 16 address bytes and scope ID. */
        IPAddress(const std::array<bytecs, 16>& addressBytes, longcs scopeId);

        /** @return true if this is an IPv4 address. */
        [[nodiscard]] bool getIsIPv4Property() const { return !isIPv6_; }
        /** @return true if this is an IPv6 address. */
        [[nodiscard]] bool getIsIPv6Property() const { return isIPv6_; }

        /** @return The address family (InterNetwork or InterNetworkV6). */
        [[nodiscard]] System::Net::Sockets::AddressFamily getAddressFamilyProperty() const;

        /**
         * @return The raw 32-bit IPv4 address in host byte order.
         * @throws System::Net::Sockets::SocketException if this is an IPv6 address.
         */
        [[nodiscard]] uint32_t getAddressProperty() const;

        /**
         * @return The IPv6 scope ID.
         * @throws System::Net::Sockets::SocketException if this is an IPv4 address.
         */
        [[nodiscard]] longcs getScopeIdProperty() const;
        /**
         * Sets the IPv6 scope ID.
         * @throws System::Net::Sockets::SocketException if this is an IPv4 address.
         */
        void setScopeIdProperty(longcs value);

        /** @return The dotted-decimal (IPv4) or colon-hex (IPv6) string representation. */
        [[nodiscard]] std::string ToString() const;

        /** @return A copy of the address as raw bytes (4 bytes for IPv4, 16 for IPv6). */
        [[nodiscard]] std::vector<bytecs> GetAddressBytes() const;

        /** @return true if the address is an IPv6 multicast address. */
        [[nodiscard]] bool getIsIPv6MulticastProperty() const;
        /** @return true if the address is an IPv6 link-local address. */
        [[nodiscard]] bool getIsIPv6LinkLocalProperty() const;
        /** @return true if the address is an IPv6 site-local address. */
        [[nodiscard]] bool getIsIPv6SiteLocalProperty() const;
        /** @return true if the address is an IPv6 Teredo address. */
        [[nodiscard]] bool getIsIPv6TeredoProperty() const;
        /** @return true if the address is an IPv6 unique-local address. */
        [[nodiscard]] bool getIsIPv6UniqueLocalProperty() const;
        /** @return true if the address is an IPv4 address mapped into IPv6 form (::FFFF:a.b.c.d). */
        [[nodiscard]] bool getIsIPv4MappedToIPv6Property() const;

        /** Maps an IPv4 address to its IPv6 equivalent (::FFFF:a.b.c.d); returns *this if already IPv6. */
        [[nodiscard]] IPAddress MapToIPv6() const;
        /** Maps the last 32 bits of an IPv6 address to an IPv4 address; returns *this if already IPv4. */
        [[nodiscard]] IPAddress MapToIPv4() const;

        /** Equality operator: compares address family, bits, and (for IPv6) scope ID. */
        bool operator==(const IPAddress& o) const;
        /** Inequality operator. */
        bool operator!=(const IPAddress& o) const { return !(*this == o); }

        /** @return A hash code combining the address bits (and scope ID, for IPv6). */
        [[nodiscard]] intcs GetHashCode() const;

        /** @return true if @p address is a loopback address (127.0.0.0/8 or ::1). */
        [[nodiscard]] static bool IsLoopback(const IPAddress& address);

        /**
         * Parses a dotted-decimal IPv4 or colon-hex IPv6 string.
         * @throws System::FormatException if @p s is not a valid IP address.
         */
        [[nodiscard]] static IPAddress Parse(const std::string& s);

        /** Attempts to parse @p s as an IPv4 or IPv6 address; returns false without throwing on failure. */
        [[nodiscard]] static bool TryParse(const std::string& s, IPAddress& address);

        /** Reverses the byte order of a 16-bit value. */
        [[nodiscard]] static shortcs HostToNetworkOrder(shortcs host);
        /** Reverses the byte order of a 32-bit value. */
        [[nodiscard]] static intcs HostToNetworkOrder(intcs host);
        /** Reverses the byte order of a 64-bit value. */
        [[nodiscard]] static longcs HostToNetworkOrder(longcs host);
        /** Reverses the byte order of a 16-bit value. */
        [[nodiscard]] static shortcs NetworkToHostOrder(shortcs network) { return HostToNetworkOrder(network); }
        /** Reverses the byte order of a 32-bit value. */
        [[nodiscard]] static intcs NetworkToHostOrder(intcs network) { return HostToNetworkOrder(network); }
        /** Reverses the byte order of a 64-bit value. */
        [[nodiscard]] static longcs NetworkToHostOrder(longcs network) { return HostToNetworkOrder(network); }

        static const IPAddress Any;          ///< 0.0.0.0 — listens on all IPv4 interfaces.
        static const IPAddress Loopback;      ///< 127.0.0.1
        static const IPAddress Broadcast;     ///< 255.255.255.255
        static const IPAddress None;          ///< 255.255.255.255 (same as Broadcast)
        static const IPAddress IPv6Any;       ///< :: — listens on all IPv6 interfaces.
        static const IPAddress IPv6Loopback;  ///< ::1
        static const IPAddress IPv6None;      ///< :: (same as IPv6Any)
    };

} // namespace System::Net
