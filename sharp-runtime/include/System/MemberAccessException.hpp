// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <exception>
#include "System/SystemException.hpp"

namespace System {

    /**
     * @brief The exception that is thrown when an attempt to access a class member fails.
     *
     * C++ counterpart of .NET System.MemberAccessException.
     * Base class for FieldAccessException, MethodAccessException, and MissingMemberException.
     */
    class MemberAccessException : public SystemException {
    public:
        /**
         * @brief Initializes a new instance with a default message.
         *
         * C++ counterpart of .NET MemberAccessException().
         */
        MemberAccessException()
            : SystemException("Cannot access this member.") {
            setHResultProperty(static_cast<SharpRuntime::intcs>(0x8013151A)); // COR_E_MEMBERACCESS
        }

        /**
         * @brief Initializes a new instance with the specified error message.
         *
         * C++ counterpart of .NET MemberAccessException(string).
         */
        explicit MemberAccessException(const std::string& message)
            : SystemException(message) {
            setHResultProperty(static_cast<SharpRuntime::intcs>(0x8013151A)); // COR_E_MEMBERACCESS
        }

        /**
         * @brief Initializes a new instance with the specified message and inner exception.
         *
         * C++ counterpart of .NET MemberAccessException(string, Exception).
         * @param message A description of the error.
         * @param inner   The exception that caused this exception.
         */
        MemberAccessException(const std::string& message, std::exception_ptr inner)
            : SystemException(message, std::move(inner)) {
            setHResultProperty(static_cast<SharpRuntime::intcs>(0x8013151A)); // COR_E_MEMBERACCESS
        }
    };

} // namespace System
