// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Created by robertvokac on 6/7/25.
//

#include "System/NotImplementedException.hpp"

namespace System {

    namespace {
        constexpr const char* DefaultNotImplementedMessage =
            "The method or operation is not implemented.";
    }

    NotImplementedException::NotImplementedException()
        : SystemException(DefaultNotImplementedMessage) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80004001)); // E_NOTIMPL
    }

    NotImplementedException::NotImplementedException(const char* message)
        : SystemException(message) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80004001)); // E_NOTIMPL
    }

    NotImplementedException::NotImplementedException(const std::string& message)
        : SystemException(message) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80004001)); // E_NOTIMPL
    }

    NotImplementedException::NotImplementedException(const std::string& message, std::exception_ptr inner)
        : SystemException(message, std::move(inner)) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80004001)); // E_NOTIMPL
    }

} // namespace System
