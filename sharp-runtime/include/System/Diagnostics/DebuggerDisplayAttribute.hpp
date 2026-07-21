// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/Attribute.hpp"

namespace System::Diagnostics {

    /**
     * @brief Determines how a class or field is displayed in the debugger variable windows.
     * 
     * C++ counterpart of .NET System.Diagnostics.DebuggerDisplayAttribute.
     */
    class DebuggerDisplayAttribute : public System::Attribute {
        std::string value_;
        std::string name_;
        std::string type_;
        std::string target_;

    public:
        /**
         * @brief Constructs the attribute with the display format string.
         * @param value Format string shown in the debugger value column.
         */
        explicit DebuggerDisplayAttribute(const std::string& value) : value_(value) {}

        /** @return The display format string shown in the debugger value column. */
        [[nodiscard]] const std::string& getValueProperty()  const { return value_; }
        /** @return The name shown in the debugger name column; empty means use default. */
        [[nodiscard]] const std::string& getNameProperty()   const { return name_; }
        /** @return The type string shown in the debugger type column; empty means use default. */
        [[nodiscard]] const std::string& getTypeProperty()   const { return type_; }
        /** @return The target type name this attribute applies to; empty means the attributed type. */
        [[nodiscard]] const std::string& getTargetProperty() const { return target_; }

        /** @brief Sets the name shown in the debugger name column. */
        void setNameProperty(const std::string& v)   { name_ = v; }
        /** @brief Sets the type string shown in the debugger type column. */
        void setTypeProperty(const std::string& v)   { type_ = v; }
        /** @brief Sets the target type name this attribute applies to. */
        void setTargetProperty(const std::string& v) { target_ = v; }
    };

} // namespace System::Diagnostics
