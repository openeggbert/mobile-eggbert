// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/NotSupportedException.hpp"

namespace System {

    /**
     * @brief The exception thrown when a feature does not run on a particular platform.
     *
     * C++ counterpart of .NET System.PlatformNotSupportedException.
     */
    class PlatformNotSupportedException : public NotSupportedException {
    public:
        /**
         * @brief Initializes a new instance with the default platform-not-supported message.
         *
         * C++ counterpart of .NET PlatformNotSupportedException().
         */
        PlatformNotSupportedException() : NotSupportedException("Operation is not supported on this platform.") {
            setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131539)); // COR_E_PLATFORMNOTSUPPORTED
        }

        /**
         * @brief Initializes a new instance with the specified error message.
         *
         * C++ counterpart of .NET PlatformNotSupportedException(string).
         */
        explicit PlatformNotSupportedException(const std::string& message) : NotSupportedException(message) {
            setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131539)); // COR_E_PLATFORMNOTSUPPORTED
        }

        /**
         * @brief Initializes a new instance with the specified message and inner exception.
         *
         * C++ counterpart of .NET PlatformNotSupportedException(string, Exception).
         */
        PlatformNotSupportedException(const std::string& message, std::exception_ptr inner)
            : NotSupportedException(message, std::move(inner)) {
            setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131539)); // COR_E_PLATFORMNOTSUPPORTED
        }
    };

} // namespace System
