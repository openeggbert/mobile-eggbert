// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/HashCode.hpp"

namespace System::Net::Sockets {

    /**
     * @brief Contains information for a socket's linger time — the amount of time it will
     * remain after Close() if data remains to be sent.
     *
     * C++ counterpart of .NET System.Net.Sockets.LingerOption.
     */
    class LingerOption {
        bool enabled_ = false;
        SharpRuntime::intcs lingerTime_ = 0;

    public:
        LingerOption(bool enable, SharpRuntime::intcs seconds) : enabled_(enable), lingerTime_(seconds) {}

        /** @return true if lingering after Close() is enabled. */
        [[nodiscard]] bool getEnabledProperty() const { return enabled_; }
        /** @brief Enables or disables lingering after Close(). */
        void setEnabledProperty(bool value) { enabled_ = value; }

        /** @return The amount of time, in seconds, to remain connected after Close(). */
        [[nodiscard]] SharpRuntime::intcs getLingerTimeProperty() const { return lingerTime_; }
        /** @brief Sets the amount of time, in seconds, to remain connected after Close(). */
        void setLingerTimeProperty(SharpRuntime::intcs value) { lingerTime_ = value; }

        [[nodiscard]] bool Equals(const LingerOption& other) const {
            return enabled_ == other.enabled_ && lingerTime_ == other.lingerTime_;
        }
        bool operator==(const LingerOption& o) const { return Equals(o); }
        bool operator!=(const LingerOption& o) const { return !Equals(o); }

        /** @brief C++ counterpart of .NET LingerOption.GetHashCode() -- HashCode.Combine(Enabled, LingerTime). */
        [[nodiscard]] SharpRuntime::intcs GetHashCode() const {
            return System::HashCode::Combine(enabled_, lingerTime_);
        }
    };

} // namespace System::Net::Sockets
