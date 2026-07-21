// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Attribute.hpp"
#include "System/Runtime/CompilerServices/MethodImplOptions.hpp"

namespace System::Runtime::CompilerServices {

    /**
     * Specifies the details of how a method is implemented (e.g. inlining hints).
     * 
     * Partial C++ counterpart of .NET System.Runtime.CompilerServices.MethodImplAttribute.
     */
    class MethodImplAttribute : public System::Attribute {
        MethodImplOptions value_;
    public:
        /** @param value The MethodImplOptions flags controlling the implementation. */
        explicit MethodImplAttribute(MethodImplOptions value) : value_(value) {}

        /**
         * Constructs from a raw integer (cast to MethodImplOptions).
         * @param value Integer representation of MethodImplOptions flags.
         */
        explicit MethodImplAttribute(int16_t value) : value_(static_cast<MethodImplOptions>(value)) {}

        /** @return The MethodImplOptions value. */
        [[nodiscard]] MethodImplOptions getValueProperty() const { return value_; }
    };

} // namespace System::Runtime::CompilerServices
