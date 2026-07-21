// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Attribute.hpp"

namespace System {

    /**
     * @brief Indicates that a method will allow a variable number of arguments in its invocation.
     *
     * C++ counterpart of .NET System.ParamArrayAttribute.
     * In C# this attribute is applied to the last parameter of a method to mark it
     * as a params array. In C++ the convention is to use variadic templates or
     * initializer_list instead; this attribute class exists solely for API compatibility.
     */
    class ParamArrayAttribute : public Attribute {
    public:
        ParamArrayAttribute() = default;
    };

} // namespace System
