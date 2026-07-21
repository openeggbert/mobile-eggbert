// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include <vector>
#include "System/Net/IPAddress.hpp"
#include "System/Net/IPEndPoint.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Net::Sockets {

    using SharpRuntime::intcs;

    /**
     * @brief Provides UDP network services.
     *
     * Partial C++ counterpart of .NET System.Net.Sockets.UdpClient.
     * Uses Winsock2 on Windows, POSIX sockets on Linux/macOS,
     * and throws PlatformNotSupportedException on Emscripten.
     *
     * @note Status: Implemented — Windows (Winsock2) and POSIX (Linux/macOS).
     */
    class UdpClient {
        int              fd_        = -1;
        Net::IPEndPoint  remote_{Net::IPAddress::Any, 0};
        bool             hasRemote_ = false;

    public:
        /** @brief Creates a UDP socket without binding to a specific port. */
        UdpClient();

        /** @brief Creates a UDP socket bound to the given local port. */
        explicit UdpClient(intcs port);

        /** @brief Creates a UDP socket bound to the given local endpoint. */
        explicit UdpClient(const Net::IPEndPoint& localEP);

        ~UdpClient();

        // Not copyable: fd_ is a raw owned socket handle closed by the destructor -- an implicit
        // shallow copy (previously allowed) lets two instances' destructors both close the same
        // fd, either failing silently or, if the fd was reused in between, closing a handle this
        // instance doesn't own. Matches Socket's own established copy-deletion.
        UdpClient(const UdpClient&) = delete;
        UdpClient& operator=(const UdpClient&) = delete;

        /** @brief Sets the default remote host/port for subsequent Send calls. */
        void Connect(const std::string& hostname, intcs port);

        /** @brief Sets the default remote endpoint for subsequent Send calls. */
        void Connect(const Net::IPEndPoint& remoteEP);

        /** @brief Sends a datagram to the default remote endpoint (must call Connect first). */
        intcs Send(const std::vector<SharpRuntime::bytecs>& dgram, intcs bytes);

        /** @brief Receives a UDP datagram; fills remoteEP with the sender's endpoint. */
        std::vector<SharpRuntime::bytecs> Receive(Net::IPEndPoint& remoteEP);

        /** @brief Closes the underlying socket. */
        void Close();

        /** @brief Returns true when the socket is open. */
        [[nodiscard]] bool getClientProperty() const { return fd_ >= 0; }
    };

} // namespace System::Net::Sockets
