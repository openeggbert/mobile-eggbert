// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/Net/IPAddress.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Net {

    using SharpRuntime::intcs;

    /**
     * Represents an IP network with an IPAddress containing the network prefix and an
     * int defining the prefix length.
     *
     * C++ counterpart of .NET System.Net.IPNetwork.
     *
     * @note This type disallows arbitrary IP-address/prefix-length CIDR pairs.
     * BaseAddress is always normalized so all bits after the network prefix are zero.
     * @note The Span/UTF8-based Parse/TryParse/TryFormat overloads are not ported (no
     * Span<T>/UTF8-string idiom in this runtime).
     */
    class IPNetwork {
        IPAddress baseAddress_ = IPAddress::Any;
        intcs prefixLength_ = 0;

        static intcs getMaxPrefixLength(const IPAddress& baseAddress);
        static IPAddress clearNonPrefixBits(const IPAddress& baseAddress, intcs prefixLength);

    public:
        /** Default: BaseAddress = 0.0.0.0, PrefixLength = 0. */
        IPNetwork() = default;

        /**
         * Constructs an IPNetwork, normalizing @p baseAddress so bits after the prefix are zero.
         * @throws std::out_of_range if @p prefixLength is negative or exceeds the address family's bit width.
         */
        IPNetwork(const IPAddress& baseAddress, intcs prefixLength);

        /** @return The IPAddress representing the network's prefix. */
        [[nodiscard]] const IPAddress& getBaseAddressProperty() const { return baseAddress_; }
        /** @return The length of the network prefix, in bits. */
        [[nodiscard]] intcs getPrefixLengthProperty() const { return prefixLength_; }

        /** @return true if @p address falls within this network. */
        [[nodiscard]] bool Contains(const IPAddress& address) const;

        /** @return CIDR notation, e.g. "192.168.1.0/24". */
        [[nodiscard]] std::string ToString() const;

        /** Equality comparison: same base address and prefix length. */
        [[nodiscard]] bool Equals(const IPNetwork& other) const {
            return prefixLength_ == other.prefixLength_ && baseAddress_ == other.baseAddress_;
        }
        bool operator==(const IPNetwork& o) const { return Equals(o); }
        bool operator!=(const IPNetwork& o) const { return !Equals(o); }

        /** @return A hash code combining the base address and prefix length. */
        [[nodiscard]] intcs GetHashCode() const;

        /**
         * Parses CIDR notation ("address/prefixLength") into an IPNetwork.
         * @throws System::FormatException if @p s is not valid CIDR notation.
         */
        [[nodiscard]] static IPNetwork Parse(const std::string& s);

        /** Attempts to parse @p s as CIDR notation; returns false without throwing on failure. */
        [[nodiscard]] static bool TryParse(const std::string& s, IPNetwork& result);
    };

} // namespace System::Net
