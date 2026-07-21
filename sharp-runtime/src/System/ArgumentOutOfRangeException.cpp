// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/ArgumentOutOfRangeException.hpp"

namespace System {

    ArgumentOutOfRangeException::ArgumentOutOfRangeException()
        : ArgumentException("Specified argument was out of the range of valid values.") {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131502)); // COR_E_ARGUMENTOUTOFRANGE
    }

    static const char* kDefaultMsg = "Specified argument was out of the range of valid values.";

    ArgumentOutOfRangeException::ArgumentOutOfRangeException(const char* paramName)
        : ArgumentException(kDefaultMsg, paramName ? paramName : "") {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131502)); // COR_E_ARGUMENTOUTOFRANGE
    }

    ArgumentOutOfRangeException::ArgumentOutOfRangeException(const std::string& paramName)
        : ArgumentException(kDefaultMsg, paramName) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131502)); // COR_E_ARGUMENTOUTOFRANGE
    }

    ArgumentOutOfRangeException::ArgumentOutOfRangeException(const std::string& message,
                                                             std::exception_ptr inner)
        : ArgumentException(message, std::move(inner)) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131502)); // COR_E_ARGUMENTOUTOFRANGE
    }

    ArgumentOutOfRangeException::ArgumentOutOfRangeException(const std::string& paramName,
                                                             const std::string& message)
        : ArgumentException(message, paramName) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131502)); // COR_E_ARGUMENTOUTOFRANGE
    }

    // Verified against ArgumentOutOfRangeException.cs's Message property override: real .NET
    // appends "\r\nActual value was {actualValue}." AFTER the "(Parameter 'x')" marker that the
    // base ArgumentException.Message already produces -- this is a defining, always-present
    // feature of ArgumentOutOfRangeException's message, not an optional extra, and every one of
    // the ThrowIfXxx static helpers below routes through this exact constructor. The previous
    // implementation stored actualValue_ (so getActualValueProperty() worked) but never wove it
    // into the actual message text, so every exception thrown from this constructor -- and by
    // extension every ThrowIfZero/ThrowIfNegative/etc. call anywhere in this codebase -- was
    // missing this suffix entirely.
    ArgumentOutOfRangeException::ArgumentOutOfRangeException(const std::string& paramName,
                                                             const std::string& actualValue,
                                                             const std::string& message)
        : ArgumentException(
              AppendParamNameSuffix(message, paramName) + "\nActual value was " + actualValue + ".",
              paramName, AlreadyComposedTag{}),
          actualValue_(actualValue) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131502)); // COR_E_ARGUMENTOUTOFRANGE
    }

} // namespace System
