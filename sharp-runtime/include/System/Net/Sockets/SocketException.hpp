// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/ComponentModel/Win32Exception.hpp"
#include "System/Net/Sockets/SocketError.hpp"
#include "System/Net/Sockets/detail/ErrnoTranslation.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#if !defined(_WIN32) && !defined(__EMSCRIPTEN__)
#include <cerrno>
#endif

namespace System::Net::Sockets {

    using SharpRuntime::intcs;

    /**
     * @brief Provides socket exceptions to the application.
     *
     * C++ counterpart of .NET System.Net.Sockets.SocketException.
     */
    class SocketException : public System::ComponentModel::Win32Exception {
        SocketError errorCode_;

    public:
        /**
         * @brief Creates a new instance using the last socket-related OS error.
         * C++ counterpart of .NET's parameterless SocketException() constructor, which captures
         * `Interop.Sys.GetLastErrorInfo()` on Unix (or `Marshal.GetLastPInvokeError()` on
         * Windows). This port is POSIX-only for System::Net::Sockets (documented in CLAUDE.md),
         * so this reads `errno` and reuses the same TranslateErrno() helper every other socket
         * call site in this codebase already uses to turn a native error into a SocketError.
         */
        SocketException()
#if !defined(_WIN32) && !defined(__EMSCRIPTEN__)
            : SocketException(SharpRuntimeDetail::Net::Sockets::TranslateErrno(errno)) {}
#else
            : SocketException(SocketError::SocketError) {}
#endif

        /** Creates a new instance with the specified error code (interpreted as a SocketError value). */
        explicit SocketException(intcs errorCode)
            : System::ComponentModel::Win32Exception(errorCode),
              errorCode_(static_cast<SocketError>(errorCode)) {}

        /** Creates a new instance with the specified error code and message. */
        SocketException(intcs errorCode, const std::string& message)
            : System::ComponentModel::Win32Exception(errorCode, message),
              errorCode_(static_cast<SocketError>(errorCode)) {}

        /** Creates a new instance from a SocketError value. */
        explicit SocketException(SocketError socketError)
            : System::ComponentModel::Win32Exception(static_cast<int>(socketError)),
              errorCode_(socketError) {}

        /** Creates a new instance from a SocketError value with a custom message. */
        SocketException(SocketError socketError, const std::string& message)
            : System::ComponentModel::Win32Exception(static_cast<int>(socketError), message),
              errorCode_(socketError) {}

        /** @return The SocketError value for this exception. */
        [[nodiscard]] SocketError getSocketErrorCodeProperty() const { return errorCode_; }

        /** @return The native error code (same as Win32Exception's NativeErrorCode). */
        [[nodiscard]] intcs getErrorCodeProperty() const { return getNativeErrorCodeProperty(); }
    };

} // namespace System::Net::Sockets
