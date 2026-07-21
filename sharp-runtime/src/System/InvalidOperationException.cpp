// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/InvalidOperationException.hpp"

namespace System {

    static constexpr const char* kDefaultMsg =
        "Operation is not valid due to the current state of the object.";

    InvalidOperationException::InvalidOperationException()
        : SystemException(kDefaultMsg) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131509)); // COR_E_INVALIDOPERATION
    }

    InvalidOperationException::InvalidOperationException(const char* message)
        : SystemException(message) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131509)); // COR_E_INVALIDOPERATION
    }

    InvalidOperationException::InvalidOperationException(const std::string& message)
        : SystemException(message) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131509)); // COR_E_INVALIDOPERATION
    }

    InvalidOperationException::InvalidOperationException(
        const std::string& message, std::exception_ptr innerException)
        : SystemException(message, std::move(innerException)) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131509)); // COR_E_INVALIDOPERATION
    }

} // namespace System