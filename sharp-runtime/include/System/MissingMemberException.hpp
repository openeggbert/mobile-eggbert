// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/MemberAccessException.hpp"

namespace System {

    /**
     * @brief The exception that is thrown when there is an attempt to dynamically
     * access a class member that does not exist or that is not declared as public.
     *
     * C++ counterpart of .NET System.MissingMemberException.
     */
    class MissingMemberException : public MemberAccessException {
    public:
        /**
         * @brief Initializes a new instance with the default missing-member message.
         *
         * C++ counterpart of .NET MissingMemberException().
         */
        MissingMemberException()
            : MemberAccessException("Attempted to access a missing member.") {
            setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131512)); // COR_E_MISSINGMEMBER
        }

        /**
         * @brief Initializes a new instance with the specified message.
         *
         * C++ counterpart of .NET MissingMemberException(string).
         */
        explicit MissingMemberException(const std::string& message)
            : MemberAccessException(message) {
            setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131512)); // COR_E_MISSINGMEMBER
        }

        /**
         * @brief Initializes a new instance with the class and member name.
         *
         * C++ counterpart of .NET MissingMemberException(string className, string memberName).
         * Matches .NET's dynamic Message format ("Member '{ClassName}.{MemberName}' not found.").
         */
        MissingMemberException(const std::string& className, const std::string& memberName)
            : MemberAccessException("Member '" + className + "." + memberName + "' not found.") {
            setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131512)); // COR_E_MISSINGMEMBER
        }

        /**
         * @brief Initializes a new instance with a message and an inner exception.
         *
         * C++ counterpart of .NET MissingMemberException(string, Exception).
         */
        MissingMemberException(const std::string& message, std::exception_ptr inner)
            : MemberAccessException(message, std::move(inner)) {
            setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131512)); // COR_E_MISSINGMEMBER
        }
    };

} // namespace System
