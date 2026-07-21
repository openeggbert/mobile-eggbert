// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/ArgumentException.hpp"
#include <algorithm>
#include <cctype>

namespace System {

    static std::string appendParamName(const std::string& msg, const std::string& paramName) {
        if (paramName.empty()) return msg;
        return msg + " (Parameter '" + paramName + "')";
    }

    ArgumentException::ArgumentException()
        : SystemException("Value does not fall within the expected range.") {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80070057)); // COR_E_ARGUMENT
    }

    ArgumentException::ArgumentException(const char* message)
        : SystemException(message ? message : "") {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80070057)); // COR_E_ARGUMENT
    }

    ArgumentException::ArgumentException(const std::string& message)
        : SystemException(message) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80070057)); // COR_E_ARGUMENT
    }

    ArgumentException::ArgumentException(const std::string& message,
                                         std::exception_ptr innerException)
        : SystemException(message, std::move(innerException)) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80070057)); // COR_E_ARGUMENT
    }

    ArgumentException::ArgumentException(const char* message, const char* paramName)
        : SystemException(appendParamName(message ? message : "", paramName ? paramName : "")),
          paramName_(paramName ? paramName : "") {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80070057)); // COR_E_ARGUMENT
    }

    ArgumentException::ArgumentException(const std::string& message,
                                         const std::string& paramName)
        : SystemException(appendParamName(message, paramName)),
          paramName_(paramName) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80070057)); // COR_E_ARGUMENT
    }

    ArgumentException::ArgumentException(const std::string& message,
                                         const std::string& paramName,
                                         std::exception_ptr innerException)
        : SystemException(appendParamName(message, paramName), std::move(innerException)),
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

    void ArgumentException::ThrowIfNullOrEmpty(const std::string& argument,
                                               const std::string& paramName) {
        if (argument.empty()) {
            throw ArgumentException("The value cannot be null or empty.", paramName);
        }
    }

    void ArgumentException::ThrowIfNullOrWhiteSpace(const std::string& argument,
                                                    const std::string& paramName) {
        bool allSpace = argument.empty() ||
            std::all_of(argument.begin(), argument.end(),
                        [](unsigned char c) { return std::isspace(c); });
        if (allSpace) {
            throw ArgumentException(
                "The value cannot be null, empty, or consist only of white-space characters.",
                paramName);
        }
    }

} // namespace System
