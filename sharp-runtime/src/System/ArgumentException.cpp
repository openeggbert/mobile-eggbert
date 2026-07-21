// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/ArgumentException.hpp"

namespace System {

    static std::string appendParamName(const std::string& msg, const std::string& paramName) {
        if (paramName.empty()) return msg;
        return msg + " (Parameter '" + paramName + "')";
    }

    ArgumentException::ArgumentException(const char* message)
        : SystemException(message ? message : "") {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80070057)); // COR_E_ARGUMENT
    }

    ArgumentException::ArgumentException(const char* message, const char* paramName)
        : SystemException(appendParamName(message ? message : "", paramName ? paramName : "")),
          paramName_(paramName ? paramName : "") {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80070057)); // COR_E_ARGUMENT
    }

    ArgumentException::ArgumentException(const std::string& message, const std::string& paramName)
        : SystemException(appendParamName(message, paramName)),
          paramName_(paramName) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80070057)); // COR_E_ARGUMENT
    }

    ArgumentException::ArgumentException(const std::string& composedMessage,
                                         const std::string& paramName,
                                         AlreadyComposedTag)
        : SystemException(composedMessage), paramName_(paramName) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80070057)); // COR_E_ARGUMENT
    }

    std::string ArgumentException::AppendParamNameSuffix(const std::string& message,
                                                          const std::string& paramName) {
        return appendParamName(message, paramName);
    }

} // namespace System
