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

    ArgumentNullException::ArgumentNullException(const char* paramName)
        : ArgumentException(makeMsg(paramName).c_str(), paramName) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80004003)); // E_POINTER
    }

} // namespace System
