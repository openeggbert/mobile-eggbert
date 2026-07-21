// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Net/EndPoint.hpp"

namespace System::Net::Sockets {

    /**
     * @brief The result of a Socket::ReceiveFromAsync() operation.
     *
     * C++ counterpart of .NET System.Net.Sockets.SocketReceiveFromResult (a struct with public
     * mutable fields in .NET — reproduced as plain public members here, not getXxx/setXxx
     * properties, since there is no property to convert).
     */
    struct SocketReceiveFromResult {
        SharpRuntime::intcs ReceivedBytes = 0;
        std::shared_ptr<System::Net::EndPoint> RemoteEndPoint;
    };

} // namespace System::Net::Sockets
