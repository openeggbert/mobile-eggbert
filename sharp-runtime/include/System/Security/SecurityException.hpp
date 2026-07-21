// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <exception>
#include <string>
#include "System/SystemException.hpp"

namespace System::Security {

    /**
     * @brief The exception thrown when a security error is detected.
     *
     * C++ counterpart of .NET System.Security.SecurityException.
     *
     * @note Reduced scope: the CAS-era members (`PermissionType`, `PermissionState`,
     * `GrantedSet`, `RefusedSet`, `Demanded`, `Denied`, `PermitOnly`, `Url`, `Method`,
     * `Zone`) are not reproduced — Code Access Security is dead code even in modern .NET
     * (its policy/permission-set infrastructure is a permanent deviation in this codebase, see
     * the many `System.Security.*` types already marked `ignore`), and several of those fields
     * are typed `System::Type`/`PermissionSet`, which don't exist here. The `(message,
     * permissionType)` constructor keeps a simplified string-based variant for source-level
     * compatibility with ported code that used it purely for a diagnostic message.
     */
    class SecurityException : public System::SystemException {
    public:
        /** @brief Initializes a new instance with a default message. */
        SecurityException() : System::SystemException("Security error.") {
            setHResultProperty(static_cast<SharpRuntime::intcs>(0x8013150A)); // COR_E_SECURITY
        }

        /** @brief Initializes a new instance with the specified message. */
        explicit SecurityException(const std::string& message) : System::SystemException(message) {
            setHResultProperty(static_cast<SharpRuntime::intcs>(0x8013150A)); // COR_E_SECURITY
        }

        /** @brief Initializes a new instance with the specified message and inner exception. */
        SecurityException(const std::string& message, std::exception_ptr innerException)
            : System::SystemException(message, std::move(innerException)) {
            setHResultProperty(static_cast<SharpRuntime::intcs>(0x8013150A)); // COR_E_SECURITY
        }

        /** @brief Initializes a new instance with a message describing the offending permission type by name. */
        SecurityException(const std::string& message, const std::string& permissionType)
            : System::SystemException(message + " (" + permissionType + ")") {
            setHResultProperty(static_cast<SharpRuntime::intcs>(0x8013150A)); // COR_E_SECURITY
        }
    };

} // namespace System::Security
