// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Net/EndPoint.hpp"
#include "System/Net/Sockets/IPPacketInformation.hpp"
#include "System/Net/Sockets/SocketFlags.hpp"

namespace System::Net::Sockets {

    /**
     * @brief The result of a Socket::ReceiveMessageFromAsync() operation.
     *
     * C++ counterpart of .NET System.Net.Sockets.SocketReceiveMessageFromResult (a struct with
     * public mutable fields in .NET — reproduced as plain public members here).
     */
    struct SocketReceiveMessageFromResult {
        SharpRuntime::intcs ReceivedBytes = 0;
        System::Net::Sockets::SocketFlags SocketFlags = System::Net::Sockets::SocketFlags::None;
        std::shared_ptr<System::Net::EndPoint> RemoteEndPoint;
        IPPacketInformation PacketInformation;
    };

} // namespace System::Net::Sockets
