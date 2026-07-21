// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/InvalidCastException.hpp"
namespace System {
    InvalidCastException::InvalidCastException() : SystemException("Specified cast is not valid.") {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80004002)); // COR_E_INVALIDCAST
    }
    InvalidCastException::InvalidCastException(const char* message) : SystemException(message) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80004002)); // COR_E_INVALIDCAST
    }
    InvalidCastException::InvalidCastException(const std::string& message) : SystemException(message) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80004002)); // COR_E_INVALIDCAST
    }
    InvalidCastException::InvalidCastException(const std::string& message, std::exception_ptr innerException)
        : SystemException(message, std::move(innerException)) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80004002)); // COR_E_INVALIDCAST
    }
    InvalidCastException::InvalidCastException(const std::string& message, SharpRuntime::intcs errorCode)
        : SystemException(message) {
        setHResultProperty(errorCode);
    }
} // namespace System
