// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/ArgumentOutOfRangeException.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Net::NetworkInformation {

    /**
     * @brief Controls the TTL and fragmentation options for a Ping request.
     *
     * C++ counterpart of .NET System.Net.NetworkInformation.PingOptions.
     */
    class PingOptions {
        SharpRuntime::intcs ttl_ = 128;
        bool dontFragment_ = false;

    public:
        /** @brief Constructs with the default TTL (128) and fragmentation allowed. */
        PingOptions() = default;

        /**
         * @brief Constructs with the given TTL and fragmentation setting.
         * @throws System::ArgumentOutOfRangeException if @p ttl is not positive.
         */
        PingOptions(SharpRuntime::intcs ttl, bool dontFragment) : ttl_(ttl), dontFragment_(dontFragment) {
            System::ArgumentOutOfRangeException::ThrowIfNegativeOrZero(ttl, "ttl");
        }

        /** @return The time-to-live value. */
        [[nodiscard]] SharpRuntime::intcs getTtlProperty() const { return ttl_; }
        /**
         * Sets the time-to-live value.
         * @throws System::ArgumentOutOfRangeException if @p value is not positive.
         */
        void setTtlProperty(SharpRuntime::intcs value) {
            System::ArgumentOutOfRangeException::ThrowIfNegativeOrZero(value, "value");
            ttl_ = value;
        }

        /** @return true if packets must not be fragmented. */
        [[nodiscard]] bool getDontFragmentProperty() const { return dontFragment_; }
        /** @brief Sets whether packets must not be fragmented. */
        void setDontFragmentProperty(bool value) { dontFragment_ = value; }
    };

} // namespace System::Net::NetworkInformation
