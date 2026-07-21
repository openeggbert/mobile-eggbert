// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/NullReferenceException.hpp"

namespace System {

    NullReferenceException::NullReferenceException()
        : SystemException("Object reference not set to an instance of an object.") {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80004003)); // E_POINTER
    }

    NullReferenceException::NullReferenceException(const char* message)
        : SystemException(message) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80004003)); // E_POINTER
    }

    NullReferenceException::NullReferenceException(const std::string& message)
        : SystemException(message) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80004003)); // E_POINTER
    }

    NullReferenceException::NullReferenceException(
        const std::string& message, std::exception_ptr innerException)
        : SystemException(message, std::move(innerException)) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80004003)); // E_POINTER
    }

} // namespace System
