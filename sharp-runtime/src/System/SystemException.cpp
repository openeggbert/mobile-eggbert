// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Created by robertvokac on 6/5/25.
//

#include "System/SystemException.hpp"

namespace System {

    SystemException::SystemException()
        : Exception("System error.") {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131501)); // COR_E_SYSTEM
    }

    SystemException::SystemException(const char* str)
        : Exception(str) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131501)); // COR_E_SYSTEM
    }

    SystemException::SystemException(const std::string& str)
        : Exception(str) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131501)); // COR_E_SYSTEM
    }

    SystemException::SystemException(const std::string& str, std::exception_ptr inner)
        : Exception(str, inner) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131501)); // COR_E_SYSTEM
    }

} // namespace System
