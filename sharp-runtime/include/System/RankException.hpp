// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/SystemException.hpp"

namespace System {

    /**
     * @brief The exception that is thrown when an array with the wrong number of dimensions is passed to a method.
     *
     * C++ counterpart of .NET System.RankException.
     */
    class RankException : public SystemException {
    public:
        /** @brief Initializes a new instance with the default rank-mismatch message. C++ counterpart of .NET RankException(). */
        RankException() : SystemException("Attempted to operate on an array with the incorrect number of dimensions.") {
            setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131517)); // COR_E_RANK
        }

        /** @brief Initializes a new instance with the specified error message. C++ counterpart of .NET RankException(string). */
        explicit RankException(const std::string& message) : SystemException(message) {
            setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131517)); // COR_E_RANK
        }

        /** @brief Initializes a new instance with a message and inner exception. C++ counterpart of .NET RankException(string, Exception). */
        RankException(const std::string& message, std::exception_ptr inner)
            : SystemException(message, std::move(inner)) {
            setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131517)); // COR_E_RANK
        }
    };

} // namespace System
