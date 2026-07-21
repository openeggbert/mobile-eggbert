// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/NotSupportedException.hpp"

namespace System {

    namespace {
        constexpr const char* DefaultMsg = "Specified method is not supported.";
    }

    NotSupportedException::NotSupportedException()
        : SystemException(DefaultMsg) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131515)); // COR_E_NOTSUPPORTED
    }

    NotSupportedException::NotSupportedException(const char* message)
        : SystemException(message) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131515)); // COR_E_NOTSUPPORTED
    }

    NotSupportedException::NotSupportedException(const std::string& message)
        : SystemException(message) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131515)); // COR_E_NOTSUPPORTED
    }

    NotSupportedException::NotSupportedException(const std::string& message,
                                                 std::exception_ptr innerException)
        : SystemException(message, std::move(innerException)) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131515)); // COR_E_NOTSUPPORTED
    }

} // namespace System
