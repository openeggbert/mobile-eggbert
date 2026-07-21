// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Net::NetworkInformation {

    /**
     * @brief Represents a physical (MAC) hardware address.
     *
     * C++ counterpart of .NET System.Net.NetworkInformation.PhysicalAddress.
     */
    class PhysicalAddress {
        std::vector<SharpRuntime::bytecs> address_;
        mutable SharpRuntime::intcs hash_ = 0;

        static bool tryGetValidSegmentLength(const std::string& address, char delimiter, SharpRuntime::intcs& value);

    public:
        /** @brief The zero-length "no address" instance. */
        static const PhysicalAddress None;

        /** @brief Constructs from raw address bytes. */
        explicit PhysicalAddress(const std::vector<SharpRuntime::bytecs>& address) : address_(address) {}

        /** @return A hash code combining all address bytes (never 0, matching .NET's sentinel handling). */
        [[nodiscard]] SharpRuntime::intcs GetHashCode() const;

        /** Equality: same length and same address bytes. */
        [[nodiscard]] bool Equals(const PhysicalAddress& other) const;
        bool operator==(const PhysicalAddress& o) const { return Equals(o); }
        bool operator!=(const PhysicalAddress& o) const { return !Equals(o); }

        /** @return The address formatted as unpunctuated uppercase hex (e.g. "001122AABBCC"). */
        [[nodiscard]] std::string ToString() const;

        /** @return A copy of the raw address bytes. */
        [[nodiscard]] std::vector<SharpRuntime::bytecs> GetAddressBytes() const { return address_; }

        /**
         * @brief Parses @p address, accepting hyphen-, colon-, or dot-delimited hex segments, or a
         * plain unpunctuated hex string.
         * @throws System::FormatException if @p address is not a valid physical address string.
         */
        [[nodiscard]] static PhysicalAddress Parse(const std::string& address);

        /** Attempts to parse @p address; returns false without throwing on failure. */
        [[nodiscard]] static bool TryParse(const std::string& address, PhysicalAddress& value);
    };

} // namespace System::Net::NetworkInformation
