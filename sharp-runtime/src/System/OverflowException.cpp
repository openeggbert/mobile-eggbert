// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/OverflowException.hpp"

namespace System {

    OverflowException::OverflowException(const char* str)
        : ArithmeticException(str) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131516)); // COR_E_OVERFLOW
    }

} // namespace System
