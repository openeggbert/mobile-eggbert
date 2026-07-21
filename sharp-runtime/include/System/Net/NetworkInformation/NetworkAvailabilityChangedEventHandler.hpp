// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <functional>
#include "System/Net/NetworkInformation/NetworkAvailabilityEventArgs.hpp"

namespace System::Net::NetworkInformation {

    /**
     * @brief Represents the method that handles the NetworkChange.NetworkAvailabilityChanged event.
     *
     * C++ counterpart of .NET System.Net.NetworkInformation.NetworkAvailabilityChangedEventHandler.
     */
    using NetworkAvailabilityChangedEventHandler = std::function<void(void* sender, NetworkAvailabilityEventArgs& e)>;

} // namespace System::Net::NetworkInformation
