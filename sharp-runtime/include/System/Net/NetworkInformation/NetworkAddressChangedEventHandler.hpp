// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <functional>
#include "System/EventArgs.hpp"

namespace System::Net::NetworkInformation {

    /**
     * @brief Represents the method that handles the NetworkChange.NetworkAddressChanged event.
     *
     * C++ counterpart of .NET System.Net.NetworkInformation.NetworkAddressChangedEventHandler.
     */
    using NetworkAddressChangedEventHandler = std::function<void(void* sender, System::EventArgs& e)>;

} // namespace System::Net::NetworkInformation
