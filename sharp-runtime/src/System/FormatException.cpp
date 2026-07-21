// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/FormatException.hpp"

namespace System {

    namespace {
        constexpr SharpRuntime::intcs CorEFormat = static_cast<SharpRuntime::intcs>(0x80131537); // COR_E_FORMAT
    }

    FormatException::FormatException(const char* message)
        : SystemException(message) {
        setHResultProperty(CorEFormat);
    }

} // namespace System
