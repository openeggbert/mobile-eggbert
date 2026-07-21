// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Net::Http {

    /**
     * @brief Defines error categories representing the reason for HttpRequestException or HttpIOException.
     *
     * C++ counterpart of .NET System.Net.Http.HttpRequestError.
     */
    enum class HttpRequestError {
        /** A generic or unknown error occurred. */
        Unknown = 0,
        /** The DNS name resolution failed. */
        NameResolutionError,
        /** A transport-level failure occurred while connecting to the remote endpoint. */
        ConnectionError,
        /** An error occurred during the TLS handshake. */
        SecureConnectionError,
        /** An HTTP/2 or HTTP/3 protocol error occurred. */
        HttpProtocolError,
        /** Extended CONNECT for WebSockets over HTTP/2 is not supported by the peer. */
        ExtendedConnectNotSupported,
        /** Cannot negotiate the HTTP version requested. */
        VersionNegotiationError,
        /** The authentication failed. */
        UserAuthenticationError,
        /** An error occurred while establishing a connection to the proxy tunnel. */
        ProxyTunnelError,
        /** An invalid or malformed response has been received. */
        InvalidResponse,
        /** The response ended prematurely. */
        ResponseEnded,
        /** The response exceeded a pre-configured limit. */
        ConfigurationLimitExceeded,
    };

} // namespace System::Net::Http
