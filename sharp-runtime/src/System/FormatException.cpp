// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/FormatException.hpp"

namespace System {

    namespace {
        constexpr const char* DefaultMsg = "One of the identified items was in an invalid format.";
    }

    namespace {
        constexpr SharpRuntime::intcs CorEFormat = static_cast<SharpRuntime::intcs>(0x80131537); // COR_E_FORMAT
    }

    FormatException::FormatException()
        : SystemException(DefaultMsg) {
        setHResultProperty(CorEFormat);
    }

    FormatException::FormatException(const char* message)
        : SystemException(message) {
        setHResultProperty(CorEFormat);
    }

    FormatException::FormatException(const std::string& message)
        : SystemException(message) {
        setHResultProperty(CorEFormat);
    }

    FormatException::FormatException(const std::string& message, std::exception_ptr inner)
        : SystemException(message, std::move(inner)) {
        setHResultProperty(CorEFormat);
    }

} // namespace System
