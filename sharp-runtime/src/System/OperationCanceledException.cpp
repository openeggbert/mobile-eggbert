// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/OperationCanceledException.hpp"

namespace System {

    OperationCanceledException::OperationCanceledException()
        : SystemException("The operation was canceled.") {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x8013153B)); // COR_E_OPERATIONCANCELED
    }

    OperationCanceledException::OperationCanceledException(const char* message)
        : SystemException(message) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x8013153B)); // COR_E_OPERATIONCANCELED
    }

    OperationCanceledException::OperationCanceledException(const std::string& message)
        : SystemException(message) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x8013153B)); // COR_E_OPERATIONCANCELED
    }

    OperationCanceledException::OperationCanceledException(
        const std::string& message, std::exception_ptr innerException)
        : SystemException(message, std::move(innerException)) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x8013153B)); // COR_E_OPERATIONCANCELED
    }

    OperationCanceledException::OperationCanceledException(const Threading::CancellationToken& token)
        : SystemException("The operation was canceled."), cancellationToken_(token) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x8013153B)); // COR_E_OPERATIONCANCELED
    }

    OperationCanceledException::OperationCanceledException(const std::string& message,
                                                             const Threading::CancellationToken& token)
        : SystemException(message), cancellationToken_(token) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x8013153B)); // COR_E_OPERATIONCANCELED
    }

    OperationCanceledException::OperationCanceledException(const std::string& message,
                                                             std::exception_ptr innerException,
                                                             const Threading::CancellationToken& token)
        : SystemException(message, std::move(innerException)), cancellationToken_(token) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x8013153B)); // COR_E_OPERATIONCANCELED
    }

} // namespace System
