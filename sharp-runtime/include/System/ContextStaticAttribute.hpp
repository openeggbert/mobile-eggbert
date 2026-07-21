// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Attribute.hpp"

namespace System {

    /**
     * @brief Indicates that the value of a static field is unique for a particular context.
     *
     * C++ counterpart of .NET System.ContextStaticAttribute.
     * In .NET this attribute may only be applied to static fields
     * (AttributeTargets.Field, Inherited = false).
     * In this port the attribute is a plain marker class with no runtime enforcement.
     */
    class ContextStaticAttribute : public Attribute {
    public:
        /**
         * @brief Initializes a new instance of the ContextStaticAttribute class.
         *
         * C++ counterpart of .NET ContextStaticAttribute().
         */
        ContextStaticAttribute() = default;
    };

} // namespace System
