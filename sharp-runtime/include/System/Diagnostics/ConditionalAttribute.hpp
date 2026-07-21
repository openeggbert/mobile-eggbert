// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/Attribute.hpp"

namespace System::Diagnostics {

    /**
     * @brief Marks a method as callable only when a specified compilation symbol is defined.
     * 
     * C++ counterpart of .NET System.Diagnostics.ConditionalAttribute.
     */
    class ConditionalAttribute : public System::Attribute {
        std::string conditionString_;
    public:
        /**
         * @brief Constructs the attribute with the given conditional compilation symbol.
         * @param conditionString The symbol name that controls whether the method is called.
         */
        explicit ConditionalAttribute(const std::string& conditionString)
            : conditionString_(conditionString) {}

        /** @return The conditional compilation symbol associated with this attribute. */
        [[nodiscard]] const std::string& getConditionStringProperty() const { return conditionString_; }
    };

} // namespace System::Diagnostics
