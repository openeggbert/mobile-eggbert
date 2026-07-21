// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/ArgumentNullException.hpp"

namespace System {

    namespace {
        constexpr const char* DefaultMsg = "Value cannot be null.";
        std::string makeMsg(const char* paramName) {
            return std::string(DefaultMsg) + " (Parameter '" + paramName + "')";
        }
    }

    ArgumentNullException::ArgumentNullException()
        : ArgumentException(DefaultMsg) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80004003)); // E_POINTER
    }

    ArgumentNullException::ArgumentNullException(const char* paramName)
        : ArgumentException(makeMsg(paramName).c_str(), paramName) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80004003)); // E_POINTER
    }

    ArgumentNullException::ArgumentNullException(const char* paramName, const char* message)
        : ArgumentException(message, paramName) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80004003)); // E_POINTER
    }

    ArgumentNullException::ArgumentNullException(const std::string& paramName)
        : ArgumentException(makeMsg(paramName.c_str()), paramName) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80004003)); // E_POINTER
    }

    ArgumentNullException::ArgumentNullException(const std::string& paramName, const std::string& message)
        : ArgumentException(message, paramName) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80004003)); // E_POINTER
    }

    ArgumentNullException::ArgumentNullException(const std::string& message, std::exception_ptr innerException)
        : ArgumentException(message, std::move(innerException)) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80004003)); // E_POINTER
    }

} // namespace System
