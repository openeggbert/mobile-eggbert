// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Net/Sockets/SocketError.hpp"

#if !defined(_WIN32) && !defined(__EMSCRIPTEN__)
#include <cerrno>
#endif

// Not a public System::* API — shared POSIX errno -> SocketError translation used by every
// System.Net.Sockets source file that constructs a SocketException from a native error code
// (Socket.cpp, TcpClient.cpp, UdpClient.cpp, NetworkStream.cpp). Verified against
// SocketErrorPal.Unix.cs's GetSocketErrorForNativeError: real .NET translates POSIX errno into
// the WSA-numbered SocketError space (SocketError's values are all 10000+X, matching Winsock's
// WSAE* numbering) before constructing a SocketException. On Windows, WSAGetLastError() already
// returns WSA-numbered codes that share SocketError's own numbering, so no translation is
// needed there — only the POSIX branch of this header does anything.
namespace SharpRuntimeDetail::Net::Sockets {

#if !defined(_WIN32) && !defined(__EMSCRIPTEN__)
    inline System::Net::Sockets::SocketError TranslateErrno(int err) {
        using System::Net::Sockets::SocketError;
        switch (err) {
            case EACCES: return SocketError::AccessDenied;
            case EADDRINUSE: return SocketError::AddressAlreadyInUse;
            case EADDRNOTAVAIL: return SocketError::AddressNotAvailable;
            case EAFNOSUPPORT: return SocketError::AddressFamilyNotSupported;
            case EAGAIN: return SocketError::WouldBlock; // EAGAIN == EWOULDBLOCK on Linux/macOS
            case EALREADY: return SocketError::AlreadyInProgress;
            case EBADF: return SocketError::OperationAborted;
#ifdef ECANCELED
            case ECANCELED: return SocketError::OperationAborted;
#endif
            case ECONNABORTED: return SocketError::ConnectionAborted;
            case ECONNREFUSED: return SocketError::ConnectionRefused;
            case ECONNRESET: return SocketError::ConnectionReset;
            case EDESTADDRREQ: return SocketError::DestinationAddressRequired;
            case EFAULT: return SocketError::Fault;
            case EHOSTDOWN: return SocketError::HostDown;
            case ENXIO: return SocketError::HostNotFound; // not perfect, but closest match available
            case EHOSTUNREACH: return SocketError::HostUnreachable;
            case EINPROGRESS: return SocketError::InProgress;
            case EINTR: return SocketError::Interrupted;
            case EINVAL: return SocketError::InvalidArgument;
            case EISCONN: return SocketError::IsConnected;
            case EMFILE: return SocketError::TooManyOpenSockets;
            case EMSGSIZE: return SocketError::MessageSize;
            case ENETDOWN: return SocketError::NetworkDown;
            case ENETRESET: return SocketError::NetworkReset;
            case ENETUNREACH: return SocketError::NetworkUnreachable;
            case ENFILE: return SocketError::TooManyOpenSockets;
            case ENOBUFS: return SocketError::NoBufferSpaceAvailable;
#ifdef ENODATA
            case ENODATA: return SocketError::NoData;
#endif
            case ENOENT: return SocketError::AddressNotAvailable;
            case ENOPROTOOPT: return SocketError::ProtocolOption;
            case ENOTCONN: return SocketError::NotConnected;
            case ENOTSOCK: return SocketError::NotSocket;
#ifdef ENOTSUP
            case ENOTSUP: return SocketError::OperationNotSupported;
#endif
            case EPERM: return SocketError::AccessDenied;
            case EPIPE: return SocketError::Shutdown;
#ifdef EPFNOSUPPORT
            case EPFNOSUPPORT: return SocketError::ProtocolFamilyNotSupported;
#endif
            case EPROTONOSUPPORT: return SocketError::ProtocolNotSupported;
            case EPROTOTYPE: return SocketError::ProtocolType;
#ifdef ESOCKTNOSUPPORT
            case ESOCKTNOSUPPORT: return SocketError::SocketNotSupported;
#endif
            case ESHUTDOWN: return SocketError::Disconnecting;
            case 0: return SocketError::Success;
            case ETIMEDOUT: return SocketError::TimedOut;
            default: return SocketError::SocketError; // unknown native error, treat as generic
        }
    }
#endif

} // namespace SharpRuntimeDetail::Net::Sockets
