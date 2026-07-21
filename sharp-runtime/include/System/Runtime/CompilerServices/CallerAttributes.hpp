// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/Attribute.hpp"

namespace System::Runtime::CompilerServices {

    // C++ equivalents: use __FUNCTION__, __FILE__, __LINE__ directly.

    /**
     * Marks a parameter to receive the caller's member name at compile time.
     * In C++ use __FUNCTION__ / __func__ instead.
     */
    class CallerMemberNameAttribute : public System::Attribute {};

    /**
     * Marks a parameter to receive the caller's source file path at compile time.
     * In C++ use __FILE__ instead.
     */
    class CallerFilePathAttribute   : public System::Attribute {};

    /**
     * Marks a parameter to receive the caller's source line number at compile time.
     * In C++ use __LINE__ instead.
     */
    class CallerLineNumberAttribute : public System::Attribute {};

    /** Marks a parameter to receive the source text of a specific argument at compile time. */
    class CallerArgumentExpressionAttribute : public System::Attribute {
        std::string parameterName_;
    public:
        /** @param parameterName Name of the parameter whose argument expression to capture. */
        explicit CallerArgumentExpressionAttribute(const std::string& parameterName)
            : parameterName_(parameterName) {}

        /** @return The name of the target parameter. */
        [[nodiscard]] const std::string& getParameterNameProperty() const { return parameterName_; }
    };

} // namespace System::Runtime::CompilerServices
