// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <any>
#include <string>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Attribute.hpp"

namespace System::ComponentModel {

    using SharpRuntime::intcs;
    using SharpRuntime::longcs;

    /** Specifies the default value for a property. */
    class DefaultValueAttribute : public System::Attribute {
        std::any value_;
    public:
        /** @param v Default value as bool. */
        explicit DefaultValueAttribute(bool v)        : value_(v) {}
        /** @param v Default value as int. */
        explicit DefaultValueAttribute(intcs v)       : value_(v) {}
        /** @param v Default value as long. */
        explicit DefaultValueAttribute(longcs v)      : value_(v) {}
        /** @param v Default value as double. */
        explicit DefaultValueAttribute(double v)      : value_(v) {}
        /** @param v Default value as float. */
        explicit DefaultValueAttribute(float v)       : value_(v) {}
        /** @param v Default value as char. */
        explicit DefaultValueAttribute(char v)        : value_(v) {}
        /** @param v Default value as string. */
        explicit DefaultValueAttribute(const std::string& v) : value_(v) {}
        /** @param v Default value as a type-erased std::any. */
        explicit DefaultValueAttribute(const std::any& v) : value_(v) {}

        /** @return The default value as a type-erased std::any. */
        [[nodiscard]] const std::any& getValueProperty() const { return value_; }
    };

} // namespace System::ComponentModel
