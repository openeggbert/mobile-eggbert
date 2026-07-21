// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/EventArgs.hpp"

namespace System::Net::NetworkInformation {

    /**
     * @brief Provides data for the NetworkChange.NetworkAvailabilityChanged event.
     *
     * C++ counterpart of .NET System.Net.NetworkInformation.NetworkAvailabilityEventArgs.
     */
    class NetworkAvailabilityEventArgs : public System::EventArgs {
        bool isAvailable_;

    public:
        /** @brief Constructs with the given availability state. */
        explicit NetworkAvailabilityEventArgs(bool isAvailable) : isAvailable_(isAvailable) {}

        /** @return true if the network is available. */
        [[nodiscard]] bool getIsAvailableProperty() const { return isAvailable_; }
    };

} // namespace System::Net::NetworkInformation
