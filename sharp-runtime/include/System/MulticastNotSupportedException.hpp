// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/SystemException.hpp"

namespace System {

    /**
     * @brief The exception thrown when there is an attempt to combine two delegates based
     * on the Delegate type instead of the MulticastDelegate type.
     *
     * C++ counterpart of .NET System.MulticastNotSupportedException.
     */
    class MulticastNotSupportedException final : public SystemException {
    public:
        /**
         * @brief Initializes a new instance with the default multicast message.
         *
         * C++ counterpart of .NET MulticastNotSupportedException().
         */
        MulticastNotSupportedException() : SystemException("Attempted to combine delegates that are not multicast.") {
            setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131514)); // COR_E_MULTICASTNOTSUPPORTED
        }

        /**
         * @brief Initializes a new instance with the specified error message.
         *
         * C++ counterpart of .NET MulticastNotSupportedException(string).
         */
        explicit MulticastNotSupportedException(const std::string& message) : SystemException(message) {
            setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131514)); // COR_E_MULTICASTNOTSUPPORTED
        }

        /**
         * @brief Initializes a new instance with the specified message and inner exception.
         *
         * C++ counterpart of .NET MulticastNotSupportedException(string, Exception).
         */
        MulticastNotSupportedException(const std::string& message, std::exception_ptr innerException)
            : SystemException(message, std::move(innerException)) {
            setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131514)); // COR_E_MULTICASTNOTSUPPORTED
        }
    };

} // namespace System
