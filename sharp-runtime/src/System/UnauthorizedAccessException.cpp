// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Created by robertvokac on 6/5/25.
//

#include "System/UnauthorizedAccessException.hpp"

namespace System {

    UnauthorizedAccessException::UnauthorizedAccessException()
        : SystemException("Attempted to perform an unauthorized operation.") {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80070005)); // COR_E_UNAUTHORIZEDACCESS
    }

    UnauthorizedAccessException::UnauthorizedAccessException(const char* str)
        : SystemException(str) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80070005)); // COR_E_UNAUTHORIZEDACCESS
    }

    UnauthorizedAccessException::UnauthorizedAccessException(const std::string& str)
        : SystemException(str) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80070005)); // COR_E_UNAUTHORIZEDACCESS
    }

    UnauthorizedAccessException::UnauthorizedAccessException(const std::string& str, std::exception_ptr inner)
        : SystemException(str, std::move(inner)) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80070005)); // COR_E_UNAUTHORIZEDACCESS
    }

} // namespace System