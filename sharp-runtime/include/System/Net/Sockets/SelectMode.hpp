// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Net::Sockets {

    /**
     * @brief Specifies the mode for polling the status of a socket.
     *
     * C++ counterpart of .NET System.Net.Sockets.SelectMode.
     */
    enum class SelectMode {
        SelectRead = 0,
        SelectWrite = 1,
        SelectError = 2,
    };

} // namespace System::Net::Sockets
