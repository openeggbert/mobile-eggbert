// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/Attribute.hpp"

namespace System::Diagnostics {

    /**
     * @brief Specifies the display proxy for a type shown in the debugger.
     * 
     * C++ counterpart of .NET System.Diagnostics.DebuggerTypeProxyAttribute.
     */
    class DebuggerTypeProxyAttribute : public System::Attribute {
        std::string proxyTypeName_;
        std::string target_;

    public:
        /**
         * @brief Constructs the attribute with the proxy type name.
         * @param typeName Fully-qualified name of the proxy type used by the debugger.
         */
        explicit DebuggerTypeProxyAttribute(const std::string& typeName) : proxyTypeName_(typeName) {}

        /** @return Fully-qualified name of the proxy type. */
        [[nodiscard]] const std::string& getProxyTypeNameProperty() const { return proxyTypeName_; }
        /** @return Target type name this proxy applies to; empty if not set. */
        [[nodiscard]] const std::string& getTargetProperty()        const { return target_; }
        /** @brief Sets the target type name this proxy applies to. */
        void setTargetProperty(const std::string& v) { target_ = v; }
    };

} // namespace System::Diagnostics
