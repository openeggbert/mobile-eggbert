// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Net::NetworkInformation {

    /**
     * @brief Specifies the operational status of a network interface.
     *
     * C++ counterpart of .NET System.Net.NetworkInformation.OperationalStatus.
     */
    enum class OperationalStatus {
        Up = 1,
        Down,
        Testing,
        Unknown,
        Dormant,
        NotPresent,
        LowerLayerDown
    };

} // namespace System::Net::NetworkInformation
