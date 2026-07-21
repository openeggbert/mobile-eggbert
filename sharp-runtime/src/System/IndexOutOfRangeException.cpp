// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/IndexOutOfRangeException.hpp"

namespace System {

    namespace {
        constexpr const char* DefaultMsg = "Index was outside the bounds of the array.";
    }

    IndexOutOfRangeException::IndexOutOfRangeException()
        : SystemException(DefaultMsg) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131508)); // COR_E_INDEXOUTOFRANGE
    }

    IndexOutOfRangeException::IndexOutOfRangeException(const char* message)
        : SystemException(message) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131508)); // COR_E_INDEXOUTOFRANGE
    }

    IndexOutOfRangeException::IndexOutOfRangeException(const std::string& message)
        : SystemException(message) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131508)); // COR_E_INDEXOUTOFRANGE
    }

    IndexOutOfRangeException::IndexOutOfRangeException(const std::string& message,
                                                       std::exception_ptr innerException)
        : SystemException(message, std::move(innerException)) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131508)); // COR_E_INDEXOUTOFRANGE
    }

} // namespace System
